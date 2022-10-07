# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Filesystem REQUIRED)

set(SRC_DIR "${CMAKE_SOURCE_DIR}/third_party/libprocinfo")

add_library(libprocinfo STATIC)
file(GLOB libprocinfo_source_files ${SRC_DIR}/*.cpp)
list(FILTER libprocinfo_source_files EXCLUDE REGEX "_(test|fuzzer|benchmark)\.cpp$")

target_sources(libprocinfo PRIVATE ${libprocinfo_source_files})
target_include_directories(libprocinfo SYSTEM PUBLIC ${SRC_DIR}/include/)
target_link_libraries(libprocinfo PUBLIC libbase)

if(NOT MSVC)
  target_compile_options(libprocinfo PRIVATE
    -Wno-error=format=
    -Wno-error=unknown-pragmas
  )
endif()