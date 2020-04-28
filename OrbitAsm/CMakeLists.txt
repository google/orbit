# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitAsm CXX ASM_MASM)

set(CMAKE_MSVC_RUNTIME_LIBRARY "")
add_library(OrbitAsm STATIC)

target_sources(OrbitAsm PUBLIC OrbitAsm.h)
target_sources(OrbitAsm PRIVATE OrbitAsm.cpp)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  target_sources(OrbitAsm PRIVATE Orbit.asm)
endif()

target_link_libraries(OrbitAsm PUBLIC minhook::minhook)
target_include_directories(OrbitAsm PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(OrbitAsm PRIVATE "$<IF:$<CONFIG:Debug>,/MDd,/MD>")
