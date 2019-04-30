cmake_minimum_required (VERSION 3.0)
project (pyrogue)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
  # TODO(amitp): I don't know how to set this properly with add_compile_options
	set(EMSCRIPTEN_FLAGS "-s USE_SDL=2 -s FORCE_FILESYSTEM=1")
	set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_FLAGS}")
	set(SDL2_LIBRARIES "-s USE_SDL=2")
	# enable SDL2, and synchronous file api
	add_definitions(${EMSCRIPTEN_FLAGS})
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EMSCRIPTEN_FLAGS}")
	SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g4")
else ()
	#find_package(SDL2 REQUIRED)
	include(FindPkgConfig)
	pkg_search_module(SDL2 REQUIRED sdl2)
endif ()

include_directories(${SDL2_INCLUDE_DIRS} lib deps src)

# librogue.a
add_library(rogue STATIC 
	lib/rogue_array.c 
	lib/rogue_display.c 
	lib/rogue_filesystem.c 
	lib/rogue_index.c 
	lib/rogue_random.c 
	lib/rogue_tty.c 
	deps/miniz.c 
)
target_compile_options(rogue PRIVATE -DMINIZ_NO_STDIO -DMINIZ_NO_ZLIB_COMPATIBLE_NAMES -DMINIZ_NO_ZLIB_APIS)
target_link_libraries(rogue ${SDL2_LIBRARIES} -lm)

# micropython module rl
set(micropython_EXTRA_MODULES ${CMAKE_SOURCE_DIR}/src/modrl.c)
include(cmake/micropython.cmake)
target_link_libraries(micropython rogue)

# pyrogue
add_executable(pyrogue src/pyrogue.c)
target_compile_options(pyrogue PRIVATE -DNO_QSTR ${micropython_CFLAGS})
target_link_libraries(pyrogue micropython)

# c examples
add_executable(plasma examples/plasma.c)
target_link_libraries(plasma rogue)

