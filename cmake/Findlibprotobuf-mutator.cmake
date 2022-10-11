# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(libprotobuf-mutator STATIC)

set(DIR "${CMAKE_SOURCE_DIR}/third_party/libprotobuf-mutator")
target_include_directories(libprotobuf-mutator SYSTEM PUBLIC "${DIR}" "${DIR}/src/")
target_sources(libprotobuf-mutator PRIVATE
  ${DIR}/src/binary_format.cc
  ${DIR}/src/mutator.cc
  ${DIR}/src/text_format.cc
  ${DIR}/src/utf8_fix.cc
  ${DIR}/src/libfuzzer/libfuzzer_macro.cc
  ${DIR}/src/libfuzzer/libfuzzer_mutator.cc
)

if(NOT MSVC)
  target_compile_options(libprotobuf-mutator PRIVATE
    -Wno-error=unused-parameter
  )
endif()

target_link_libraries(libprotobuf-mutator PRIVATE protobuf::protobuf)

add_library(libprotobuf-mutator::libprotobuf-mutator ALIAS libprotobuf-mutator)
