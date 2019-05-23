cmake_minimum_required (VERSION 3.0)
project (pyrogue)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

if (EMSCRIPTEN)
  # TODO(amitp): I don't know how to set this properly with add_compile_options
	set(EMSCRIPTEN_FLAGS "-s USE_SDL=2 -s FORCE_FILESYSTEM=1 -s TOTAL_MEMORY=100MB -s USE_WEBGL2=1")
	set(CMAKE_C_FLAGS "-m32 ${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_FLAGS}")
	set(SDL2_LIBRARIES "-s USE_SDL=2")
	add_definitions(${EMSCRIPTEN_FLAGS})
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EMSCRIPTEN_FLAGS}")
	SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g4")
else ()
	include(FindPkgConfig)
	pkg_search_module(SDL2 REQUIRED sdl2)
	set(OpenGL_GL_PREFERENCE LEGACY)
	find_package(OpenGL REQUIRED)
	find_package(GLEW REQUIRED)
	set(SDLGPU_LIBRARIES ${OPENGL_gl_LIBRARY} ${GLEW_LIBRARY})
endif ()

if (WIN32)
	SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -gstabs")
endif()

#set(SDLGPU_DIR "${CMAKE_SOURCE_DIR}/deps/sdl-gpu/build/SDL_gpu-0.11.0")
#set(SDLGPU_LIBRARIES ${SDLGPU_DIR}/lib/libSDL2_gpu.a -lGL -lGLX)
set(SDLGPU_INCLUDE_DIRS 
	${CMAKE_SOURCE_DIR}/deps/sdl-gpu/include 
	${CMAKE_SOURCE_DIR}/deps/sdl-gpu/src/externals/stb_image_write
	${CMAKE_SOURCE_DIR}/deps/sdl-gpu/src/externals/glew/GL
)

include_directories(${SDL2_INCLUDE_DIRS} ${SDLGPU_INCLUDE_DIRS} lib deps src)

set(SDLGPU ${CMAKE_SOURCE_DIR}/deps/sdl-gpu/src)
# librogue.a

set(rogue_SOURCE
	lib/rogue_util.c 
	lib/rogue_array.c 
	lib/rogue_display.c 
	lib/rogue_filesystem.c 
	lib/rogue_index.c 
	lib/rogue_random.c 
	lib/rogue_tty.c 

	deps/miniz.c 
	${SDLGPU}/SDL_gpu.c
	${SDLGPU}/SDL_gpu_matrix.c
	${SDLGPU}/SDL_gpu_renderer.c
	${SDLGPU}/SDL_gpu_shapes.c
	${SDLGPU}/renderer_OpenGL_1_BASE.c
	${SDLGPU}/renderer_OpenGL_1.c
	${SDLGPU}/renderer_OpenGL_2.c
	${SDLGPU}/renderer_OpenGL_3.c
	${SDLGPU}/renderer_OpenGL_4.c
	${SDLGPU}/renderer_GLES_1.c
	${SDLGPU}/renderer_GLES_2.c
	${SDLGPU}/renderer_GLES_3.c
	${SDLGPU}/externals/stb_image/stb_image.c
	${SDLGPU}/externals/stb_image_write/stb_image_write.c
)

#if(!WIN32)
	set(rogue_SOURCE ${rogue_SOURCE} ${SDLGPU}/externals/glew/glew.c)
#endif()

add_library(rogue STATIC ${rogue_SOURCE})

if (EMSCRIPTEN)
set(SDLGPU_DEFINITIONS
	GL_GLEXT_PROTOTYPES
	SDL_GPU_DISABLE_OPENGL
	SDL_GPU_DISABLE_GLES_1
	SDL_GPU_DISABLE_GLES_2
)
elseif (WIN32)
set(SDLGPU_DEFINITIONS
	GL_GLEXT_PROTOTYPES
	SDL_GPU_DISABLE_OPENGL_1
	SDL_GPU_DISABLE_OPENGL_2
	SDL_GPU_DISABLE_OPENGL_4
	SDL_GPU_DISABLE_GLES
)
else ()
set(SDLGPU_DEFINITIONS
	GL_GLEXT_PROTOTYPES
	SDL_GPU_DISABLE_OPENGL
	SDL_GPU_DISABLE_GLES_1
	SDL_GPU_DISABLE_GLES_2
)
endif()

target_compile_definitions(rogue PRIVATE 
	MINIZ_NO_ARCHIVE_WRITING_APIS
	MINIZ_NO_STDIO
	MINIZ_NO_TIME
	MINIZ_NO_ZLIB_APIS
	MINIZ_NO_ZLIB_COMPATIBLE_NAME
	MINIZ_NO_ZLIB_COMPATIBLE_NAMES
	USE_SDLGPU
	${SDLGPU_DEFINITIONS}
)

target_link_libraries(rogue ${SDL2_LIBRARIES} ${SDLGPU_LIBRARIES} -lm)

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
add_executable(test_filesystem examples/test_filesystem.c)
target_link_libraries(test_filesystem rogue)

