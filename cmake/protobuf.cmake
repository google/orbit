# Copyright (c) 2020 The Orbit Authors. All rights reserved. Use of this source
# code is governed by a BSD-style license that can be found in the LICENSE file.

cmake_minimum_required(VERSION 3.12)

# include-what-you-use does not need the whole project to be compiled but it
# needs all the code generators to be run - like protobuf.
# The following code is overwriting the protobuf_generate function and adds
# all protobuf targets as dependencies to the global iwyu target.

include("third_party/protobuf/protobuf-generate.cmake")

if(TARGET iwyu)
  function(protobuf_generate)
    _protobuf_generate(${ARGN})

    cmake_parse_arguments(args "" "TARGET" "" "${ARGN}")
    if (args_TARGET)
      add_dependencies(iwyu ${args_TARGET})
    endif()
  endfunction()
endif()

