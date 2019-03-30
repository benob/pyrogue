/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Paul Sokolovsky
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "py/compile.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "py/stackctrl.h"
#include "extmod/misc.h"

#include "rogue.h"

STATIC void stderr_print_strn(void *env, const char *str, size_t len) {
	(void)env;
	ssize_t dummy = write(STDERR_FILENO, str, len);
	mp_uos_dupterm_tx_strn(str, len);
	(void)dummy;
}

const mp_print_t mp_stderr_print = {NULL, stderr_print_strn};

mp_obj_t execute_from_str(const char *str) {
	nlr_buf_t nlr;
	if (nlr_push(&nlr) == 0) {
		qstr src_name = 1/*MP_QSTR_*/;
		mp_lexer_t *lex = mp_lexer_new_from_str_len(src_name, str, strlen(str), false);
		mp_parse_tree_t pt = mp_parse(lex, MP_PARSE_FILE_INPUT);
		mp_obj_t module_fun = mp_compile(&pt, src_name, MP_EMIT_OPT_NONE, false);
		mp_call_function_0(module_fun);
		nlr_pop();
		return 0;
	} else {
		// uncaught exception
		mp_obj_base_t *exc = (mp_obj_t)nlr.ret_val;
		mp_obj_print_exception(&mp_stderr_print, MP_OBJ_FROM_PTR(exc));
		return (mp_obj_t)nlr.ret_val;
	}
}

#define FORCED_EXIT (0x100)
// If exc is SystemExit, return value where FORCED_EXIT bit set,
// and lower 8 bits are SystemExit value. For all other exceptions,
// return 1.
STATIC int handle_uncaught_exception(mp_obj_base_t *exc) {
    // check for SystemExit
    if (mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(exc->type), MP_OBJ_FROM_PTR(&mp_type_SystemExit))) {
        // None is an exit value of 0; an int is its value; anything else is 1
        mp_obj_t exit_val = mp_obj_exception_get_value(MP_OBJ_FROM_PTR(exc));
        mp_int_t val = 0;
        if (exit_val != mp_const_none && !mp_obj_get_int_maybe(exit_val, &val)) {
            val = 1;
        }
        return FORCED_EXIT | (val & 255);
    }

    // Report all other exceptions
    mp_obj_print_exception(&mp_stderr_print, MP_OBJ_FROM_PTR(exc));
    return 1;
}

#define LEX_SRC_STR (1)
#define LEX_SRC_VSTR (2)
#define LEX_SRC_FILENAME (3)
#define LEX_SRC_STDIN (4)

#define MP_QSTR__lt_stdin_gt_ 1
// Returns standard error codes: 0 for success, 1 for all other errors,
// except if FORCED_EXIT bit is set then script raised SystemExit and the
// value of the exit is in the lower 8 bits of the return value
STATIC int execute_from_lexer(int source_kind, const void *source, mp_parse_input_kind_t input_kind, bool is_repl) {
    //mp_hal_set_interrupt_char('c');

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        // create lexer based on source kind
        mp_lexer_t *lex;
        if (source_kind == LEX_SRC_STR) {
            const char *line = source;
            lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, line, strlen(line), false);
        } else if (source_kind == LEX_SRC_VSTR) {
            const vstr_t *vstr = source;
            lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, vstr->buf, vstr->len, false);
        } else if (source_kind == LEX_SRC_FILENAME) {
            lex = mp_lexer_new_from_file((const char*)source);
        } else { // LEX_SRC_STDIN
            lex = mp_lexer_new_from_fd(MP_QSTR__lt_stdin_gt_, 0, false);
        }

        qstr source_name = lex->source_name;

        #if MICROPY_PY___FILE__
        if (input_kind == MP_PARSE_FILE_INPUT) {
            //mp_store_global(MP_QSTR___file__, MP_OBJ_NEW_QSTR(source_name));
        }
        #endif

        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);

        #if defined(MICROPY_UNIX_COVERAGE)
        // allow to print the parse tree in the coverage build
        if (mp_verbose_flag >= 3) {
            printf("----------------\n");
            mp_parse_node_print(parse_tree.root, 0);
            printf("----------------\n");
        }
        #endif

        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, MP_EMIT_OPT_NONE, is_repl);

				// execute it
				mp_call_function_0(module_fun);
				// check for pending exception
				if (MP_STATE_VM(mp_pending_exception) != MP_OBJ_NULL) {
					mp_obj_t obj = MP_STATE_VM(mp_pending_exception);
					MP_STATE_VM(mp_pending_exception) = MP_OBJ_NULL;
					nlr_raise(obj);
				}

        //mp_hal_set_interrupt_char(-1);
        nlr_pop();
        return 0;

    } else {
        // uncaught exception
        //mp_hal_set_interrupt_char(-1);
        return handle_uncaught_exception(nlr.ret_val);
    }
}

STATIC int do_file(const char *file) {
    return execute_from_lexer(LEX_SRC_FILENAME, file, MP_PARSE_FILE_INPUT, false);
}

STATIC int do_str(const char *str) {
    return execute_from_lexer(LEX_SRC_STR, str, MP_PARSE_FILE_INPUT, false);
}

/* Usage:
 * ./pyrogue (no args) => show usage unless a game is embedded in the executable
 * ./pyrogue script.py => cd to script directory and run script.py
 * ./pyrogue archive.zip => load archive and run game.py from archive root
 * TODO ./pyrogue directory/ => cd to directory and run game.py
 * TODO ./pyrogue -embed directory output => create exectuable with embedded directory content
 * TODO ./pyrogue -extract directory => extract embedded data if it exists
 *
 * embedded executable is a zip at the end of current executable after the "__RLPY__PAYLOAD__" marker
 * TODO change to a marker and offset at the end of the executable
 */
void usage(char* arg0) {
	printf("usage: %s <game.py>|<game.zip>\n", arg0);
	exit(1);
}

int main(int argc, char** argv) {
	if(argc > 2) usage(argv[0]);

	// Initialized stack limit
	mp_stack_set_limit(40000 * (BYTES_PER_WORD / 4));
	// Initialize heap
#if MICROPY_ENABLE_GC
#define HEAP_SIZE (2 * 1024 * 1024)
	char* heap = malloc(HEAP_SIZE);

	gc_init(heap, heap + HEAP_SIZE);
#endif

#if MICROPY_ENABLE_PYSTACK
	static mp_obj_t pystack[1024];
	mp_pystack_init(pystack, &pystack[MP_ARRAY_SIZE(pystack)]);
#endif

	// Initialize interpreter
	mp_init();

	mp_obj_list_init(MP_OBJ_TO_PTR(mp_sys_path), 0);
	mp_obj_list_init(MP_OBJ_TO_PTR(mp_sys_argv), 0);

	uint32_t content_size;
	char* content;
	if(argc == 1) {
		fs_open_resources(argv[0]);
		content = fs_load_asset("game.py", &content_size);
		if(content == NULL) usage(argv[0]);
	} else if(!strcmp(argv[1] + strlen(argv[1]) - 4, ".zip")) {
		fs_open_resources(argv[1]);
		content = fs_load_asset("game.py", &content_size);
		if(content == NULL) {
			fprintf(stderr, "cannot load 'game.py' from '%s'\n", argv[1]);
			exit(1);
		}
	} else {
		FILE* fp = fopen(argv[1], "r");
		if(!fp) {
			perror(argv[1]);
			exit(1);
		}
		if(fseek(fp, 0, SEEK_END) < 0) {
			perror(argv[1]);
			exit(1);
		}
		long position = ftell(fp);
		if((int) position < 0) {
			fprintf(stderr, "%s: Not a valid python file\n", argv[1]);
			exit(1);
		}
		content_size = position;
		content = malloc(content_size + 1);
		fseek(fp, 0, SEEK_SET);
		fread(content, content_size, 1, fp);
		content[content_size] = '\0';
		fclose(fp);
		char* slash = strrchr(argv[1], '/');
		if(slash != NULL) {
			*(slash + 1) = '\0';
			fs_open_resources(argv[1]);
		}
	}

	/*if (execute_from_str(content)) {
		printf("Error\n");
	}*/
	//fprintf(stderr, "%s\n", content);
	do_str(content);
	free(content);

	mp_deinit();

#if MICROPY_ENABLE_GC
	free(heap);
#endif
}

uint mp_import_stat(const char *path) {
	struct stat st;
	if (stat(path, &st) == 0) {
		if (S_ISDIR(st.st_mode)) {
			return MP_IMPORT_STAT_DIR;
		} else if (S_ISREG(st.st_mode)) {
			return MP_IMPORT_STAT_FILE;
		}
	}
	return MP_IMPORT_STAT_NO_EXIST;
}

void nlr_jump_fail(void *val) {
	printf("FATAL: uncaught NLR %p\n", val);
	exit(1);
}
