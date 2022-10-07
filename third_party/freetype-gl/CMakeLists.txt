# Freetype GL - A C OpenGL Freetype engine
#
# Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
# file `LICENSE` for more details.

cmake_minimum_required(VERSION 2.8.12)

project(freetype-gl LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

set(CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules"
    "${CMAKE_MODULE_PATH}"
)
# The additional / is important to remove the last character from the path.
# Note that it does not matter if the OS uses / or \, because we are only
# saving the path size.
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

message(STATUS "Building for ${CMAKE_SYSTEM_NAME} target system")
message(STATUS "Building with ${CMAKE_C_COMPILER_ID} compiler")

set(freetype-gl_WITH_GLEW_DEFAULT ON)

if((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") OR
    (${CMAKE_SYSTEM_NAME} MATCHES "Android"))
    set(freetype-gl_WITH_GLEW_DEFAULT OFF)
endif()

option(freetype-gl_WITH_GLEW
    "Use the GLEW library to fetch OpenGL function pointers"
    ${freetype-gl_WITH_GLEW_DEFAULT})
option(freetype-gl_WITH_GLAD "Use the GLAD gl loader" OFF)
option(freetype-gl_USE_VAO "Use a VAO to render a vertex_buffer instance (required for forward compatible OpenGL 3.0 contexts)" OFF)
option(freetype-gl_BUILD_DEMOS "Build the freetype-gl example programs" ON)
option(freetype-gl_BUILD_APIDOC "Build the freetype-gl API documentation" ON)
option(freetype-gl_BUILD_HARFBUZZ "Build the freetype-gl harfbuzz support (experimental)" OFF)
option(freetype-gl_BUILD_MAKEFONT "Build the makefont tool" ON)
option(freetype-gl_BUILD_TESTS "Build the tests" ON)
option(freetype-gl_BUILD_SHARED "Build shared library" OFF)
option(freetype-gl_OFF_SCREEN "Build for off-screen render (build libfreetype-gl.so only without GLX, demos must disable because of missing glfw)" OFF)

include(RequireIncludeFile)
include(RequireFunctionExists)
include(CheckLibraryExists)
include(CheckSymbolExists)

require_include_file(stdbool.h HAVE_STDBOOL_H)
require_include_file(stdint.h HAVE_STDINT_H)
require_include_file(math.h HAVE_MATH_H)

check_library_exists(m cos "" HAVE_MATH_LIBRARY)

if(HAVE_MATH_LIBRARY)
    list(APPEND CMAKE_REQUIRED_LIBRARIES m)
    set(MATH_LIBRARY m)
endif()

require_function_exists(cos HAVE_COS)
require_function_exists(fabs HAVE_FABS)
require_function_exists(floor HAVE_FLOOR)
require_function_exists(fmod HAVE_FMOD)
require_function_exists(pow HAVE_POW)
require_function_exists(roundf HAVE_ROUNDF)
require_function_exists(round HAVE_ROUND)
require_function_exists(sin HAVE_SIN)
require_function_exists(sqrt HAVE_SQRT)
require_function_exists(tan HAVE_TAN)

check_symbol_exists(M_PI math.h HAVE_M_PI)

if(NOT HAVE_M_PI)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_USE_MATH_DEFINES)
    unset(HAVE_M_PI CACHE)

    check_symbol_exists(M_PI math.h HAVE_M_PI)

    if(NOT HAVE_M_PI)
        message(FATAL_ERROR "`M_PI` not defined in `math.h`.")
    else()
        add_definitions(-D_USE_MATH_DEFINES)
    endif()
endif()


if(freetype-gl_OFF_SCREEN)
    set(freetype-gl_BUILD_DEMOS OFF)

    # Build OFF screen must use FindOpenGL with GLVND
    # The only one module OpenGL are needed for libOpenGL.so
    set(OpenGL_GL_PREFERENCE GLVND)
    find_package(OpenGL REQUIRED OpenGL EGL)
    set(OPENGL_LIBRARY ${OPENGL_opengl_LIBRARY})
else()
    find_package(OpenGL REQUIRED)
endif()
find_package(Freetype REQUIRED)

if(freetype-gl_WITH_GLEW)
    set(FREETYPE_GL_USE_GLEW 1)
    find_package(GLEW REQUIRED)
endif()

if(freetype-gl_WITH_GLAD)
    set(GL_WITH_GLAD 1)
endif()

include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLEW_INCLUDE_PATH}
    ${PROJECT_BINARY_DIR}
)

if(MSVC)
    # _CRT_NONSTDC_NO_DEPRECATE -> remove warning C4996
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
endif(MSVC)

if(freetype-gl_USE_VAO)
    set(FREETYPE_GL_USE_VAO 1)
endif(freetype-gl_USE_VAO)

configure_file (
        "${PROJECT_SOURCE_DIR}/cmake/config.h.in"
        "${PROJECT_BINARY_DIR}/config.h"
)

set(FREETYPE_GL_HDR
    distance-field.h
    edtaa3func.h
    font-manager.h
    freetype-gl.h
    markup.h
    opengl.h
    platform.h
    text-buffer.h
    texture-atlas.h
    texture-font.h
    utf8-utils.h
    ftgl-utils.h
    vec234.h
    vector.h
    vertex-attribute.h
    vertex-buffer.h
    freetype-gl-errdef.h
    ${PROJECT_BINARY_DIR}/config.h
)

set(FREETYPE_GL_SRC
    distance-field.c
    edtaa3func.c
    font-manager.c
    platform.c
    text-buffer.c
    texture-atlas.c
    texture-font.c
    utf8-utils.c
    ftgl-utils.c
    vector.c
    vertex-attribute.c
    vertex-buffer.c
)

if(NOT MSVC)
	set(PKG_CONFIG_PREFIX ${CMAKE_INSTALL_PREFIX})
	set(PKG_CONFIG_EXEC_PREFIX "\${prefix}")
	set(PKG_CONFIG_LIBDIR "\${exec_prefix}/lib")
	set(PKG_CONFIG_INCLUDEDIR "\${prefix}/include")
	configure_file(freetype-gl.pc.in ${CMAKE_CURRENT_BINARY_DIR}/freetype-gl.pc @ONLY)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/freetype-gl.pc DESTINATION ${PKG_CONFIG_PREFIX}/share/pkgconfig)
endif()

if(freetype-gl_BUILD_SHARED)
    add_library(freetype-gl SHARED
        ${FREETYPE_GL_SRC}
        ${FREETYPE_GL_HDR}
    )
    SET_TARGET_PROPERTIES(
        freetype-gl
    PROPERTIES
        VERSION 0.3.2
        SOVERSION 0)
    target_link_libraries (freetype-gl
			   ${OPENGL_LIBRARY}
			   ${FREETYPE_LIBRARIES}
			   ${MATH_LIBRARY}
			   ${GLEW_LIBRARY}
			   )
else()
    add_library(freetype-gl STATIC
        ${FREETYPE_GL_SRC}
        ${FREETYPE_GL_HDR}
    )
endif()

if(freetype-gl_BUILD_MAKEFONT)
    add_executable(makefont makefont.c)

    target_link_libraries(makefont
        freetype-gl
        ${OPENGL_LIBRARY}
        ${FREETYPE_LIBRARIES}
        ${MATH_LIBRARY}
        ${GLEW_LIBRARY}
    )

    if(MSVC AND NOT (MSVC_VERSION LESS 1900))
        # prevent error LNK2019: unresolved external symbol _sprintf referenced in function __bdf_parse_properties
        # see http://stackoverflow.com/a/32418900/469659
        target_link_libraries(makefont "legacy_stdio_definitions.lib")
    endif()
endif()

install(TARGETS freetype-gl
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(FILES ${FREETYPE_GL_HDR}
  DESTINATION include/freetype-gl
  PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

if(freetype-gl_BUILD_APIDOC)
    add_subdirectory(doc)
endif()

if(freetype-gl_BUILD_HARFBUZZ)
    add_subdirectory(harfbuzz)
endif()

if(freetype-gl_BUILD_DEMOS)
    add_subdirectory(demos)
endif()

if(freetype-gl_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif(freetype-gl_BUILD_TESTS)

get_property(LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)

if ("${LIB64}" STREQUAL "TRUE")
    set(LIBSUFFIX 64)
else()
    set(LIBSUFFIX "")
endif()

set(INSTALL_LIB_DIR     lib${LIBSUFFIX} CACHE PATH "Installation directory for libraries")
mark_as_advanced(INSTALL_LIB_DIR)

install(TARGETS freetype-gl
	ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
	LIBRARY DESTINATION ${INSTALL_LIB_DIR}
	COMPONENT library)
install(FILES ${FREETYPE_GL_HDR} DESTINATION include
	COMPONENT headers)

