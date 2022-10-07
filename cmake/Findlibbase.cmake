# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set(SRC_DIR "${CMAKE_SOURCE_DIR}/third_party/libbase")

if(NOT TARGET liblog_headers)
  add_library(liblog_headers INTERFACE)
  target_include_directories(liblog_headers SYSTEM INTERFACE ${SRC_DIR}/../liblog/include/)
endif()

file(GLOB libbase_source_files ${SRC_DIR}/*.cpp)
list(FILTER libbase_source_files EXCLUDE REGEX "_(test|fuzzer|benchmark)\.cpp$")
list(FILTER libbase_source_files EXCLUDE REGEX "(test_main|test_utils)\.cpp$")
list(FILTER libbase_source_files EXCLUDE REGEX "(utf8|_windows)\.cpp$")

add_library(libbase STATIC)
target_sources(libbase PRIVATE ${libbase_source_files})
target_link_libraries(libbase PUBLIC liblog_headers)
target_include_directories(libbase SYSTEM PUBLIC ${SRC_DIR}/include/)

if(NOT MSVC)
  target_compile_options(libbase PRIVATE
    -Wno-error=unused-function
    -Wno-error=unknown-pragmas
    -Wno-error=ignored-attributes
  )
endif()
