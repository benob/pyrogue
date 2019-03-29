cmake_minimum_required (VERSION 2.6)
project (plasma)

add_library(rogue STATIC 
	lib/rogue_array.c 
	lib/rogue_display.c 
	lib/rogue_filesystem.c 
	lib/rogue_index.c 
	lib/rogue_random.c 
	lib/rogue_tty.c 
	deps/miniz.c 
)

add_executable(plasma examples/plasma.c)
target_link_libraries(plasma rogue)

add_library(micropython STATIC

)

add_executable(pyrogue src/pyrogue.c)
target_link_libraries(pyrogue micropython rogue)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(SDL2 REQUIRED)

INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS} include deps)
TARGET_LINK_LIBRARIES(rogue ${SDL2_LIBRARY} -lm)

