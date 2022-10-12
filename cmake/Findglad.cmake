# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(glad CONFIG QUIET)

if(TARGET glad::glad)
  return()
endif()

message(STATUS "GLAD not found via CMake. Using the version in third_party/...")

add_library(glad STATIC ${CMAKE_SOURCE_DIR}/third_party/glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/third_party/glad/include)
target_link_libraries(glad PUBLIC ${CMAKE_DL_LIBS})

add_library(glad::glad ALIAS glad)
