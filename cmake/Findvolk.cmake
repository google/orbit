# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(volk CONFIG QUIET)

if(TARGET volk::volk)
  return()
endif()

message(STATUS "volk not found via find_package. Trying to find on the system...")

find_path(VOLK_INC_DIR NAMES volk.h REQUIRED)

# Volk is provided as a C source file in Ubuntu which we need to compile ourselves.
add_library(volk STATIC ${VOLK_INC_DIR}/volk.c)
target_include_directories(volk INTERFACE ${VULKAN_INC_DIR})
target_link_libraries(volk PRIVATE ${CMAKE_DL_LIBS})

add_library(volk::volk ALIAS volk)
