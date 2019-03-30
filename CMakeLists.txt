cmake_minimum_required (VERSION 3.0)
project (plasma)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} include deps src)

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
target_link_libraries(rogue ${SDL2_LIBRARY} -lm)

set(micropython_EXTRA_MODULES ${CMAKE_SOURCE_DIR}/src/modrl.c)
include(cmake/micropython.cmake)
add_library(micropython_extra_modules ${micropython_EXTRA_MODULES} ${GENHDR}/qstrdefs.generated.h)
target_link_libraries(micropython micropython_extra_modules rogue)

# pyrogue
add_executable(pyrogue src/pyrogue.c)
target_compile_options(pyrogue PRIVATE -DNO_QSTR)
target_link_libraries(pyrogue micropython)

# examples
add_executable(plasma examples/plasma.c)
target_link_libraries(plasma rogue)


