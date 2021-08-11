# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

function(add_fuzzer target_name)
  # Fuzzing is (currently) only fully supported on clang. So we will
  # only enable the fuzzing-flag on clang, but we will still compile it
  # in any case, to protect the fuzz-tests from breaking. Since the
  # executable's main-function is provided by libfuzzer we have to compile
  # the code to a static library on other compilers instead.
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_executable(${target_name} ${ARGN})

    if (ENV{LIB_FUZZING_ENGINE})
      message(STATUS "Adding linker flags according to LIB_FUZZING_ENGINE env variable: $ENV{LIB_FUZZING_ENGINE}")
      set(FUZZING_OPTION $ENV{LIB_FUZZING_ENGINE})
    else()
      set(FUZZING_OPTION "-fsanitize=fuzzer")
    endif()

    target_compile_options(${target_name} PUBLIC ${FUZZING_OPTION})
    set_property(
      TARGET ${target_name}
      APPEND
      PROPERTY LINK_OPTIONS ${FUZZING_OPTION})
  else()
    add_library(${target_name} STATIC ${ARGN})
  endif()
endfunction()

# Usage example:
# add_fuzzer(ClassNameMethodNameFuzzer ClassNameMethodNameFuzzer.cpp)
# target_link_libraries(ClassNameMethodNameFuzzer ${PROJECT_NAME})
