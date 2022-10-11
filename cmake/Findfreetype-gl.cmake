# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(freetype-gl STATIC)

set(DIR "${CMAKE_SOURCE_DIR}/third_party/freetype-gl")
target_include_directories(freetype-gl PUBLIC SYSTEM "${DIR}")
target_sources(freetype-gl PRIVATE
  ${DIR}/distance-field.c
  ${DIR}/edtaa3func.c
  ${DIR}/font-manager.c
  ${DIR}/ftgl-utils.c
  ${DIR}/platform.c
  ${DIR}/text-buffer.c
  ${DIR}/texture-atlas.c
  ${DIR}/texture-font.c
  ${DIR}/utf8-utils.c
  ${DIR}/vector.c
  ${DIR}/vertex-attribute.c
  ${DIR}/vertex-buffer.c
  ${DIR}/demos/mat4.c
  ${DIR}/demos/shader.c
)

if(NOT MSVC)
 target_compile_options(freetype-gl PRIVATE
    -Wno-error=unused-parameter
    -Wno-error=unused-variable
    -Wno-error=sign-compare
    -Wno-error=float-conversion
    -Wno-error=format-nonliteral
    -Wno-error=unused-result
  )
endif()

set(FREETYPE_GL_USE_GLEW 0)
set(FREETYPE_GL_USE_VAO 0)
set(GL_WITH_GLAD 1)

set(BIN_DIR "${CMAKE_BINARY_DIR}/third_party/freetype-gl")
set(INC_DIR "${BIN_DIR}/include")
configure_file("${DIR}/cmake/config.h.in" "${INC_DIR}/config.h")

file(GLOB headers "${DIR}/*.h")
list(APPEND headers
  "${DIR}/demos/mat4.h"
  "${DIR}/demos/shader.h"
)
file(COPY ${headers} DESTINATION "${INC_DIR}/freetype-gl/")

target_include_directories(freetype-gl PUBLIC ${INC_DIR})
target_link_libraries(freetype-gl PRIVATE glad::glad freetype)

file(COPY "${DIR}/fonts/Vera.ttf" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fonts/")
file(COPY "${DIR}/shaders/v3f-t2f-c4f.frag" "${DIR}/shaders/v3f-t2f-c4f.vert"
     DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/")

add_library(freetype-gl::freetype-gl ALIAS freetype-gl)
