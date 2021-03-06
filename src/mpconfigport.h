/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Damien P. George
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

// include pyrogue module
#define PYROGUE_MOD_RL              (1)
// import modules from pyrogue assets
#define PYROGUE_IMPORT_ASSET        (1)

/*#define mp_type_textio mp_type_vfs_posix_textio
#define MICROPY_VFS_POSIX (1)
#define MICROPY_VFS (1)*/
// options to control how MicroPython is built

#define MICROPY_USE_INTERNAL_PRINTF (0) // disable internal printf
#define MICROPY_ALLOC_PATH_MAX      (PATH_MAX)
#define MICROPY_ENABLE_GC           (1)
#define MICROPY_ENABLE_FINALISER    (1) 
#ifdef __EMSCRIPTEN__
#define MICROPY_STACK_CHECK         (0)
#define MICROPY_MEM_STATS           (0)
#else
#define MICROPY_STACK_CHECK         (1)
#define MICROPY_MEM_STATS           (1)
#endif
#define MICROPY_MALLOC_USES_ALLOCATED_SIZE (1)
#define MICROPY_DEBUG_PRINTERS      (1)
#define MICROPY_READER_POSIX        (1)
#define MICROPY_KBD_EXCEPTION       (1)
#define MICROPY_HELPER_REPL         (1)
#define MICROPY_HELPER_LEXER_UNIX   (1)
#define MICROPY_ENABLE_SOURCE_LINE  (1)
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_DETAILED)
#define MICROPY_WARNINGS            (1)
#define MICROPY_ENABLE_EMERGENCY_EXCEPTION_BUF   (1)
#define MICROPY_FLOAT_IMPL          (MICROPY_FLOAT_IMPL_DOUBLE)
#define MICROPY_LONGINT_IMPL        (MICROPY_LONGINT_IMPL_LONGLONG)
#define MICROPY_STREAMS_NON_BLOCK   (0)
#define MICROPY_OPT_COMPUTED_GOTO   (0) // unsupported by llvm wasm generator (emscripten port) as of july 2019
#define MICROPY_OPT_CACHE_MAP_LOOKUP_IN_BYTECODE (1)
#define MICROPY_OPT_MPZ_BITWISE (1)
#define MICROPY_OPT_MATH_FACTORIAL (1)
#define MICROPY_CAN_OVERRIDE_BUILTINS (1)
#define MICROPY_BUILTIN_METHOD_CHECK_SELF_ARG (1)
#define MICROPY_CPYTHON_COMPAT      (1)
#define MICROPY_PY_GC               (1)
#define MICROPY_PY_GC_COLLECT_RETVAL (1)
#define MICROPY_PY_DESCRIPTORS      (1)
#define MICROPY_PY_FUNCTION_ATTRS   (1)
#define MICROPY_PY_BUILTINS_BYTEARRAY (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW (1)
#define MICROPY_PY_BUILTINS_COMPILE (1)
#define MICROPY_PY_BUILTINS_ENUMERATE (1)
#define MICROPY_PY_BUILTINS_FILTER  (1)
#define MICROPY_PY_BUILTINS_FROZENSET (1)
#define MICROPY_PY_BUILTINS_REVERSED (1)
#define MICROPY_PY_BUILTINS_SET     (1)
#define MICROPY_PY_BUILTINS_SLICE   (1)
#define MICROPY_PY_BUILTINS_STR_UNICODE (1)
#define MICROPY_PY_BUILTINS_STR_CENTER (1)
#define MICROPY_PY_BUILTINS_STR_PARTITION (1)
#define MICROPY_PY_BUILTINS_STR_SPLITLINES (1)
#define MICROPY_PY_BUILTINS_PROPERTY (1)
#define MICROPY_PY_BUILTINS_MIN_MAX (1)
#define MICROPY_PY_BUILTINS_INPUT   (0) // disabled because we live in a zip
#define MICROPY_PY_BUILTINS_POW3    (1)
#define MICROPY_PY_BUILTINS_ROUND_INT    (1)
#define MICROPY_PY_ALL_SPECIAL_METHODS (1)
#define MICROPY_PY_REVERSE_SPECIAL_METHODS (1)
#define MICROPY_PY_BUILTINS_SLICE_ATTRS (1)
#define MICROPY_PY___FILE__         (0)
#define MICROPY_PY_MICROPYTHON_MEM_INFO (0)
#define MICROPY_COMP_MODULE_CONST   (1)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN (1)
#define MICROPY_COMP_RETURN_IF_EXPR (1)
#define MICROPY_PY_ARRAY            (0)
#define MICROPY_PY_COLLECTIONS      (1)
#define MICROPY_PY_MATH             (1)
#define MICROPY_PY_CMATH            (0)
#define MICROPY_PY_IO               (1)
#define MICROPY_PY_IO_FILEIO        (1)
#define MICROPY_PY_STRUCT           (1)
#define MICROPY_PY_SYS              (1)
#define MICROPY_PY_SYS_EXIT         (1)
#define MICROPY_PY_SYS_PLATFORM     "pyrogue"
#define MICROPY_PY_SYS_MAXSIZE      (0)
#define MICROPY_PY_SYS_STDFILES     (0)
#define MICROPY_PY_UCTYPES          (1)
#define MICROPY_PY_UZLIB            (0)
#define MICROPY_PY_UJSON            (1)
#define MICROPY_PY_URE              (1)
#define MICROPY_PY_UHEAPQ           (0)
#define MICROPY_PY_UHASHLIB         (0)
#define MICROPY_PY_UBINASCII        (0)
#define MICROPY_PY_COLLECTIONS_DEQUE (1)
#define MICROPY_PY_COLLECTIONS_ORDEREDDICT (1)

#ifdef __EMSCRIPTEN__
#define MICROPY_EMIT_X64            (0) //BROKEN
#define MICROPY_EMIT_THUMB          (0) //BROKEN
#define MP_SSIZE_MAX (0x7fffffff)
#define MICROPY_EVENT_POLL_HOOK \
    do { \
        extern void mp_handle_pending(void); \
        mp_handle_pending(); \
    } while (0);

#define MICROPY_THREAD_YIELD()
/*#define MICROPY_VM_HOOK_COUNT (10)
#define MICROPY_VM_HOOK_INIT static uint vm_hook_divisor = MICROPY_VM_HOOK_COUNT;
#define MICROPY_VM_HOOK_POLL if (--vm_hook_divisor == 0) { \
        vm_hook_divisor = MICROPY_VM_HOOK_COUNT; \
        extern void mp_js_hook(void); \
        mp_js_hook(); \
    }
#define MICROPY_VM_HOOK_LOOP MICROPY_VM_HOOK_POLL
#define MICROPY_VM_HOOK_RETURN MICROPY_VM_HOOK_POLL*/
#define MP_STATE_PORT MP_STATE_VM

#endif

extern const struct _mp_obj_module_t mp_module_os;
extern const struct _mp_obj_module_t mp_module_rl;

#define MICROPY_PORT_BUILTIN_MODULES \
    { MP_ROM_QSTR(MP_QSTR_rl), MP_ROM_PTR(&mp_module_rl) }, \
    { MP_ROM_QSTR(MP_QSTR_uos), MP_ROM_PTR(&mp_module_os) }, \

#define MICROPY_PORT_ROOT_POINTERS \

#define mp_check_self(pred) if(!(pred)) mp_raise_msg(&mp_type_TypeError, "wrong type for self");

//////////////////////////////////////////
// Do not change anything beyond this line
//////////////////////////////////////////

// Define to 1 to use undertested inefficient GC helper implementation
// (if more efficient arch-specific one is not available).
#ifndef MICROPY_GCREGS_SETJMP
    #ifdef __mips__
        #define MICROPY_GCREGS_SETJMP (1)
    #else
        #define MICROPY_GCREGS_SETJMP (0)
    #endif
#endif

// type definitions for the specific machine

#ifdef __MINGW32__ // windows support

// From windows port
//#define MICROPY_PORT_INIT_FUNC      init()
//#define MICROPY_PORT_DEINIT_FUNC    deinit()

#if defined( __MINGW32__ ) && defined( __LP64__ )
typedef long mp_int_t; // must be pointer size
typedef unsigned long mp_uint_t; // must be pointer size
#elif defined ( __MINGW32__ ) && defined( _WIN64 )
#include <stdint.h>
typedef __int64 mp_int_t;
typedef unsigned __int64 mp_uint_t;
#define MP_SSIZE_MAX __INT64_MAX__
#elif defined ( _MSC_VER ) && defined( _WIN64 )
typedef __int64 mp_int_t;
typedef unsigned __int64 mp_uint_t;
#else
// These are definitions for machines where sizeof(int) == sizeof(void*),
// regardless for actual size.
typedef int mp_int_t; // must be pointer size
typedef unsigned int mp_uint_t; // must be pointer size
#endif

#define MP_ENDIANNESS_LITTLE (1)

// Cannot include <sys/types.h>, as it may lead to symbol name clashes
#if _FILE_OFFSET_BITS == 64 && !defined(__LP64__)
typedef long long mp_off_t;
#else
typedef long mp_off_t;
#endif

#if MICROPY_PY_OS_DUPTERM
#define MP_PLAT_PRINT_STRN(str, len) mp_hal_stdout_tx_strn_cooked(str, len)
void mp_hal_dupterm_tx_strn(const char *str, size_t len);
#else
#include <unistd.h>
#define MP_PLAT_PRINT_STRN(str, len) do { int ret = write(1, str, len); (void)ret; } while (0)
#define mp_hal_dupterm_tx_strn(s, l)
#endif

#define MP_STATE_PORT               MP_STATE_VM

#define MICROPY_MPHALPORT_H         "windows_mphal.h"

#include "realpath.h"
#include "init.h"
#include "sleep.h"

#define UINT_FMT "%I64u"
#define INT_FMT "%I64d"

#ifdef __GNUC__
#define MP_NOINLINE __attribute__((noinline))
#endif


#else // not __MINGW32__

#if defined( __EMSCRIPTEN__ )
#include <stdint.h>
#define UINT_FMT "%u"
#define INT_FMT "%d"
typedef int mp_int_t; // must be pointer size
typedef unsigned mp_uint_t; // must be pointer size
#elif defined( __LP64__ )
typedef long mp_int_t; // must be pointer size
typedef unsigned long mp_uint_t; // must be pointer size
#else
// These are definitions for machines where sizeof(int) == sizeof(void*),
// regardless for actual size.
typedef int mp_int_t; // must be pointer size
typedef unsigned int mp_uint_t; // must be pointer size
#endif

// Cannot include <sys/types.h>, as it may lead to symbol name clashes
#if _FILE_OFFSET_BITS == 64 && !defined(__LP64__)
typedef long long mp_off_t;
#else
typedef long mp_off_t;
#endif

#endif // end not __MINGW32__

// We need to provide a declaration/definition of alloca()
#ifdef __FreeBSD__
#include <stdlib.h>
#elif __MINGW32__
#include <malloc.h>
#else
#include <alloca.h>
#endif
