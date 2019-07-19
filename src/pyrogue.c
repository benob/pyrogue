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
#include "py/misc.h"
#include "extmod/misc.h"
#include "lib/utils/interrupt_char.h"

#include "rogue.h"

// memory for micropython
char* python_heap;

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
STATIC int execute_from_lexer(int source_kind, const void *source, uint32_t size, mp_parse_input_kind_t input_kind, bool is_repl, const char* name) {
#ifndef __EMSCRIPTEN__
    mp_hal_set_interrupt_char('c');
#endif

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        // create lexer based on source kind
        mp_lexer_t *lex;
				qstr qname = qstr_from_strn(name, strlen(name));
        if (source_kind == LEX_SRC_STR) {
            const char *line = source;
            lex = mp_lexer_new_from_str_len(qname, line, size, /*strlen(line),*/ false);
        } else if (source_kind == LEX_SRC_VSTR) {
            const vstr_t *vstr = source;
            lex = mp_lexer_new_from_str_len(qname, vstr->buf, vstr->len, false);
        } else if (source_kind == LEX_SRC_FILENAME) {
            lex = mp_lexer_new_from_file((const char*)source);
        } else { // LEX_SRC_STDIN
            lex = mp_lexer_new_from_fd(qname, 0, false);
        }
				qstr source_name = lex->source_name;

        #if MICROPY_PY___FILE__
        if (input_kind == MP_PARSE_FILE_INPUT) {
					mp_store_global(MP_QSTR___file__, MP_OBJ_NEW_QSTR(source_name));
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
#ifndef __EMSCRIPTEN__
        mp_hal_set_interrupt_char(-1);
#endif
        return handle_uncaught_exception(nlr.ret_val);
    }
}

/*STATIC int do_file(const char *file) {
    return execute_from_lexer(LEX_SRC_FILENAME, file, MP_PARSE_FILE_INPUT, false);
}*/

STATIC int do_str(const char *str, uint32_t size, const char* name) {
    return execute_from_lexer(LEX_SRC_STR, str, size, MP_PARSE_FILE_INPUT, false, name);
}

void error_handler(const char* message) {
  mp_raise_msg(&mp_type_Exception, message);
}

const char* get_manifest_value(const char* data, const char* key) {
	static char value[1024];
	if(data == NULL) return NULL;
	if(key == NULL) return NULL;
	const char* start = data;
	for(const char* end = strchr(data, '\n'); start; start = end) {
		while(*start == '\n' || *start == '\r' || *start == ' ' || *start == '\t') start++;
		const char* equals = strchr(start, '=');
		if(equals != NULL) {
			equals--;
			while(equals > start && (*equals == ' ' || *equals == '\t')) equals--;
			if(!strncmp(key, start, equals - start)) {
				equals++;
				while(*equals == ' ' || *equals == '\t' || *equals == '=') equals++;
				if(end == NULL) end = start + strlen(start) - 1;
				while(end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end--;
				int length = end - equals + 1;
				if(length > 0) {
					memcpy(value, equals, length);
					value[length] = '\0';
					return value;
				}
			}
		}
		if(end && *end) end = strchr(end + 1, '\n');
	}
	return NULL;
}

void usage(char* arg0) {
	printf("usage: %s [options]\n", arg0);
	printf("  without arguments             run game.py from embedded zip if any\n");
	printf("  <file.py>                     run script with resource path set to its directory\n");
	printf("  <zip>                         run game.py from root of zip\n");
	printf("  -embed <zip> <target-exe>     create exe embedding zip for standalone execution\n");
	printf("  -extract <zip>                extract embedd zip from executable\n");
	exit(1);
}

MP_NOINLINE int cmdline_main(int argc, char** argv);

#ifdef __EMSCRIPTEN__
static char *stack_top;

void gc_collect(void) {
    // WARNING: This gc_collect implementation doesn't try to get root
    // pointers from CPU registers, and thus may function incorrectly.
	printf("gc_collect\n"); // TODO
    jmp_buf dummy;
    if (setjmp(dummy) == 0) {
        longjmp(dummy, 1);
    }
    gc_collect_start();
    gc_collect_root((void*)stack_top, ((mp_uint_t)(void*)(&dummy + 1) - (mp_uint_t)stack_top) / sizeof(mp_uint_t));
    gc_collect_end();
}
#else // not(__EMSCRIPTEN__)
// from mp unix port, capture stack asap
int main(int argc, char** argv) {
    #if MICROPY_PY_THREAD
    mp_thread_init();
    #endif
    mp_stack_ctrl_init();
    return cmdline_main(argc, argv);
}
#endif

MP_NOINLINE void pyrogue_init(const char* resource_path) {
#ifdef __EMSCRIPTEN__
	int stack_dummy;
	stack_top = (char*)&stack_dummy;
#if MICROPY_PY_THREAD
	mp_thread_init();
#endif
	mp_stack_ctrl_init();
#endif
	// Initialized stack limit
	mp_stack_set_limit(40000 * (BYTES_PER_WORD / 4));
	// Initialize heap
#if MICROPY_ENABLE_GC
	// allocate 50M to python (used for python memory and rl arrays)
#define HEAP_SIZE (50 * 1024 * 1024)
	python_heap = malloc(HEAP_SIZE);

	gc_init(python_heap, python_heap + HEAP_SIZE);
#endif

#if MICROPY_ENABLE_PYSTACK
	static mp_obj_t pystack[1024];
	mp_pystack_init(pystack, &pystack[MP_ARRAY_SIZE(pystack)]);
#endif

	// Initialize interpreter
	mp_init();

	mp_obj_list_init(MP_OBJ_TO_PTR(mp_sys_path), 0);
	mp_obj_list_init(MP_OBJ_TO_PTR(mp_sys_argv), 0);

	rl_set_allocator(m_malloc, m_realloc, m_free);
	if(resource_path != NULL) fs_open_resources(resource_path);

}

MP_NOINLINE void pyrogue_shutdown() {
	mp_deinit();

#if MICROPY_ENABLE_GC
	free(python_heap);
#endif
}

MP_NOINLINE void pyrogue_run_string(const char* name, const char* code) {
	rl_set_error_handler(error_handler);
	do_str(code, strlen(code), name);
}

MP_NOINLINE int pyrogue_run(const char* filename) {
	char* content = NULL;
	const char* script = "game.py";
	if(!strcmp(filename + strlen(filename) - 3, ".py")) {
		char* path = strdup(filename);
		char* basename = strdup(filename);
		char* slash = strrchr(path, '/');
		if(slash == NULL) slash = strrchr(path, '\\');
		if(slash != NULL) {
			free(basename);
			basename = strdup(slash + 1);
			*(slash + 1) = '\0';
			fs_open_resources(path);
		}
		content = fs_load_asset(basename, NULL);
		free(path);
		free(basename);
	} else {
		// zip or executable with embedded zip
		fs_open_resources(filename);
		char* manifest = fs_load_asset("MANIFEST", NULL);
		if(manifest) {
			const char* value = get_manifest_value(manifest, "script");
			if(value) script = value;
			free(manifest);
		}
		content = fs_load_asset(script, NULL);
		if(!content && !strcmp(filename + strlen(filename) - 4, ".zip")) 
			rl_error("cannot load script '%s' from '%s'", script, filename);
		filename = script;
	}
	if(content != NULL) {
		pyrogue_run_string(filename, content);
		free(content);
		return 1;
	}
	return 0;
}

MP_NOINLINE void pyrogue_quit() {
	td_quit();
}

// main for command line invocation
MP_NOINLINE int cmdline_main(int argc, char** argv) {
	pyrogue_init(NULL);

	if(argc == 1) {
		if(!pyrogue_run(argv[0])) usage(argv[0]);
	} else if(argc == 2) {
    if(!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) usage(argv[0]);
    else pyrogue_run(argv[1]);
	} else if (argc == 3 && !strcmp(argv[1], "-extract")) {
		fs_extract_embed(argv[0], argv[2]);
		return 0;
	} else if (argc == 4 && !strcmp(argv[1], "-embed")) {
		fs_add_embed(argv[0], argv[2], argv[3]);
		return 0;
	} else {
		usage(argv[0]);
	}

	pyrogue_shutdown();

	return 0;
}

// support importing from assets
uint mp_import_stat(const char *path) {
	if(fs_asset_is_file(path)) {
		return MP_IMPORT_STAT_FILE; 
	} else if(fs_asset_is_directory(path)) {
		return MP_IMPORT_STAT_DIR;
	}
	return MP_IMPORT_STAT_NO_EXIST;
}

void nlr_jump_fail(void *val) {
	printf("FATAL: uncaught NLR %p\n", val);
	assert(1 == 0);
	exit(1);
}
