set(MP ${CMAKE_SOURCE_DIR}/micropython)

set(GENHDR ${CMAKE_BINARY_DIR}/genhdr)

include_directories(${MP} ${GENHDR}/..)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

if(MINGW) # windows
	set(micropython_platform_CFLAGS
		-I${MP}/ports/windows
		)
	set(micropython_platform_SOURCE 
		${MP}/ports/unix/main.c
		${MP}/ports/unix/file.c
		${MP}/ports/unix/input.c
		${MP}/ports/unix/modos.c
		${MP}/ports/unix/modmachine.c
		${MP}/ports/unix/modtime.c
		${MP}/ports/unix/gccollect.c
		${MP}/ports/windows/fmode.c  
		${MP}/ports/windows/init.c  
		${MP}/ports/windows/realpath.c
		${MP}/ports/windows/sleep.c
		${MP}/ports/windows/windows_mphal.c
		)
else(MINGW) # regular unix
	set(micropython_platform_CFLAGS
		-I${MP}/ports/unix
		${micropython_CFLAGS}
		)
	set(micropython_platform_SOURCE 
		${MP}/ports/unix/main.c
		${MP}/ports/unix/gccollect.c
		${MP}/ports/unix/unix_mphal.c
		${MP}/ports/unix/input.c
		${MP}/ports/unix/file.c
		${MP}/ports/unix/modmachine.c
		${MP}/ports/unix/modos.c
		${MP}/ports/unix/moduselect.c
		${MP}/ports/unix/alloc.c
		${MP}/ports/unix/coverage.c
		${MP}/ports/unix/fatfs_port.c
		)
endif(MINGW)

set(micropython_CFLAGS 
	-I. 
	-I${GENHDR}/.. 
	-I${CMAKE_SOURCE_DIR}/src 
	-I${CMAKE_SOURCE_DIR}/include 
	${micropython_platform_CFLAGS}
	-I${MP}/ 
	-Wall
	-Werror
	-Wpointer-arith
	-Wuninitialized
	-Wno-unused-label
	-std=gnu99
	-DUNIX
	-U_FORTIFY_SOURCE
	-Os
	)

set(micropython_regular_SOURCE
	${MP}/py/mpstate.c
	${MP}/py/malloc.c
	${MP}/py/qstr.c
	${MP}/py/vstr.c
	${MP}/py/mpprint.c
	${MP}/py/unicode.c
	${MP}/py/mpz.c
	${MP}/py/reader.c
	${MP}/py/lexer.c
	${MP}/py/parse.c
	${MP}/py/scope.c
	${MP}/py/compile.c
	${MP}/py/emitcommon.c
	${MP}/py/emitbc.c
	${MP}/py/asmbase.c
	${MP}/py/asmx64.c
	${MP}/py/asmx86.c
	${MP}/py/asmthumb.c
	${MP}/py/emitinlinethumb.c
	${MP}/py/asmarm.c
	${MP}/py/asmxtensa.c
	${MP}/py/emitinlinextensa.c
	${MP}/py/formatfloat.c
	${MP}/py/parsenumbase.c
	${MP}/py/parsenum.c
	${MP}/py/emitglue.c
	${MP}/py/persistentcode.c
	${MP}/py/runtime.c
	${MP}/py/runtime_utils.c
	${MP}/py/nativeglue.c
	${MP}/py/stackctrl.c
	${MP}/py/argcheck.c
	${MP}/py/warning.c
	${MP}/py/gc.c
	${MP}/py/vm.c
	${MP}/py/map.c
	${MP}/py/obj.c
	${MP}/py/nlr.c
	${MP}/py/nlrthumb.c
	${MP}/py/nlrx86.c
	${MP}/py/nlrx64.c
	${MP}/py/nlrxtensa.c
	${MP}/py/nlrsetjmp.c
	${MP}/py/objarray.c
	${MP}/py/objattrtuple.c
	${MP}/py/objbool.c
	${MP}/py/objboundmeth.c
	${MP}/py/objcell.c
	${MP}/py/objclosure.c
	${MP}/py/objcomplex.c
	${MP}/py/objdict.c
	${MP}/py/objenumerate.c
	${MP}/py/objexcept.c
	${MP}/py/objfilter.c
	${MP}/py/objfloat.c
	${MP}/py/objfun.c
	${MP}/py/objgenerator.c
	${MP}/py/objgetitemiter.c
	${MP}/py/objint.c
	${MP}/py/objint_longlong.c
	${MP}/py/objint_mpz.c
	${MP}/py/objlist.c
	${MP}/py/objmap.c
	${MP}/py/objmodule.c
	${MP}/py/objobject.c
	${MP}/py/objpolyiter.c
	${MP}/py/objproperty.c
	${MP}/py/objnone.c
	${MP}/py/objnamedtuple.c
	${MP}/py/objrange.c
	${MP}/py/objreversed.c
	${MP}/py/objset.c
	${MP}/py/objsingleton.c
	${MP}/py/objslice.c
	${MP}/py/objstr.c
	${MP}/py/objstrunicode.c
	${MP}/py/objstringio.c
	${MP}/py/objtuple.c
	${MP}/py/objtype.c
	${MP}/py/objzip.c
	${MP}/py/opmethods.c
	${MP}/py/sequence.c
	${MP}/py/stream.c
	${MP}/py/binary.c
	${MP}/py/builtinimport.c
	${MP}/py/builtinevex.c
	${MP}/py/modarray.c
	${MP}/py/modbuiltins.c
	${MP}/py/modcollections.c
	${MP}/py/modgc.c
	${MP}/py/modio.c
	${MP}/py/modmath.c
	${MP}/py/modcmath.c
	${MP}/py/modmicropython.c
	${MP}/py/modstruct.c
	${MP}/py/modsys.c
	${MP}/py/moduerrno.c
	${MP}/py/modthread.c
	${MP}/py/bc.c
	${MP}/py/showbc.c
	${MP}/py/repl.c
	${MP}/py/smallint.c
	${MP}/py/frozenmod.c
	${MP}/py/pystack.c
	${MP}/py/emitnx64.c
	${MP}/py/emitnx86.c
	${MP}/py/emitnthumb.c
	${MP}/py/emitnarm.c
	${MP}/py/emitnxtensa.c
	${MP}/py/scheduler.c
	${MP}/py/objdeque.c
	${MP}/py/builtinhelp.c
	${MP}/extmod/moductypes.c
	${MP}/extmod/modujson.c
	${MP}/extmod/modure.c
	${MP}/extmod/moduzlib.c
	${MP}/extmod/moduheapq.c
	${MP}/extmod/modutimeq.c
	${MP}/extmod/moduhashlib.c
	${MP}/extmod/moducryptolib.c
	${MP}/extmod/modubinascii.c
	${MP}/extmod/virtpin.c
	${MP}/extmod/machine_mem.c
	${MP}/extmod/machine_pinbase.c
	${MP}/extmod/machine_signal.c
	${MP}/extmod/machine_pulse.c
	${MP}/extmod/machine_i2c.c
	${MP}/extmod/machine_spi.c
	${MP}/extmod/modussl_axtls.c
	${MP}/extmod/modussl_mbedtls.c
	${MP}/extmod/modurandom.c
	${MP}/extmod/moduselect.c
	${MP}/extmod/moduwebsocket.c
	${MP}/extmod/modwebrepl.c
	${MP}/extmod/modframebuf.c
	${MP}/extmod/vfs.c
	${MP}/extmod/vfs_reader.c
	${MP}/extmod/vfs_posix.c
	${MP}/extmod/vfs_posix_file.c
	${MP}/extmod/vfs_fat.c
	${MP}/extmod/vfs_fat_diskio.c
	${MP}/extmod/vfs_fat_file.c
	${MP}/extmod/utime_mphal.c
	${MP}/extmod/uos_dupterm.c
	${MP}/lib/embed/abort_.c
	${MP}/lib/utils/printf.c
  ${MP}/lib/timeutils/timeutils.c
  ${MP}/py/mpconfig.h
	)

#TODO: verify that this works
set_source_files_properties(${MP}/py/gc.c PROPERTIES COMPILE_FLAGS -O3)
set_source_files_properties(${MP}/py/vm.c PROPERTIES COMPILE_FLAGS -O3)

set(micropython_SOURCE
	${micropython_regular_SOURCE}
	${micropython_platform_SOURCE}
	${micropython_EXTRA_MODULES}
	)

add_library(micropython ${micropython_regular_SOURCE} ${micropython_platform_SOURCE} ${GENHDR}/qstrdefs.generated.h)
target_compile_options(micropython PRIVATE ${micropython_CFLAGS})
target_compile_definitions(micropython PRIVATE FFCONF_H=\"${MP}/lib/oofatfs/ffconf.h\")

add_library(micropython_extra_modules ${micropython_EXTRA_MODULES} ${GENHDR}/qstrdefs.generated.h)
target_compile_options(micropython_extra_modules PRIVATE ${micropython_CFLAGS})

add_custom_command(OUTPUT ${GENHDR}/qstrdefs.generated.h
	COMMAND mkdir -p ${GENHDR}
	COMMAND python3 ${MP}/py/makeversionhdr.py ${GENHDR}/mpversion.h
	COMMAND python3 ${MP}/py/makemoduledefs.py --vpath="., .., " ${micropython_SOURCE} ${GENHDR}/moduledefs.h > ${GENHDR}/moduledefs.h
	COMMAND ${CMAKE_C_COMPILER} -E -DNO_QSTR ${micropython_CFLAGS} -DFFCONF_H='\"${MP}/lib/oofatfs/ffconf.h\"' ${micropython_SOURCE} ${CMAKE_SOURCE_DIR}/src/mpconfigport.h > ${GENHDR}/qstr.i.last
	COMMAND python3 ${MP}/py/makeqstrdefs.py split ${GENHDR}/qstr.i.last ${GENHDR}/qstr ${GENHDR}/qstrdefs.collected.h
	COMMAND python3 ${MP}/py/makeqstrdefs.py cat ${GENHDR}/qstr.i.last ${GENHDR}/qstr ${GENHDR}/qstrdefs.collected.h
	COMMAND cat ${MP}/py/qstrdefs.h ${MP}/ports/unix/qstrdefsport.h ${GENHDR}/qstrdefs.collected.h | sed [=['s/^Q(.*)/"&"/']=] | ${CMAKE_C_COMPILER} -E ${micropython_CFLAGS} -DFFCONF_H='\"${MP}/lib/oofatfs/ffconf.h\"' - | sed [=['s/^"\(Q(.*)\)"/\1/']=] > ${GENHDR}/qstrdefs.preprocessed.h
	COMMAND python3 ${MP}/py/makeqstrdata.py ${GENHDR}/qstrdefs.preprocessed.h > ${GENHDR}/qstrdefs.generated.h
	DEPENDS ${micropython_SOURCE} ${CMAKE_SOURCE_DIR}/src/mpconfigport.h
	)

