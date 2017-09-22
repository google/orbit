# Freetype GL - A C OpenGL Freetype engine
#
# Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
# file `LICENSE` for more details.

find_package(HarfBuzz REQUIRED)

include_directories(
    ${HARFBUZZ_INCLUDE_DIRS}
)

set(FREETYPE_GL_HB_HDR
    freetype-gl.h
    opengl.h
    texture-atlas.h
    texture-font.h
    vec234.h
    vector.h
    vertex-attribute.h
    vertex-buffer.h
)

set(FREETYPE_GL_HB_SRC
    texture-atlas.c
    texture-font.c
    vector.c
    vertex-attribute.c
    vertex-buffer.c
)

add_library(freetype-gl-hb STATIC
    ${FREETYPE_GL_HB_SRC}
    ${FREETYPE_GL_HB_HDR}
)
