// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FUZZING_UTILS_FUZZER_H_
#define FUZZING_UTILS_FUZZER_H_

#include <libfuzzer/libfuzzer_macro.h>

#include "OrbitBase/Logging.h"

// Use the macro `ORBIT_DEFINE_FUZZER` to define an LLVM-based fuzzer that automatically catches
// exceptions thrown by `CHECK`, `UNREACHABLE`, and other in non-fuzzing-mode aborting macros. The
// function needs to take exactly two arguments of type `uint8_t*` and `size_t`.
//
// Example:
// ORBIT_DEFINE_FUZZER(uint8_t* buf, size_t size) {
//  // Put your fuzzing code here.
// }

#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
#define ORBIT_DEFINE_FUZZER(arg1, arg2)                             \
  static int OrbitFuzzerTestFunction(arg1, arg2);                   \
  extern "C" int LLVMFuzzerTestOneInput(uint8_t* buf, size_t len) { \
    try {                                                           \
      return OrbitFuzzerTestFunction(buf, len);                     \
    } catch (const orbit_base::FuzzingException&) {                 \
    }                                                               \
    return 0;                                                       \
  }                                                                 \
  int OrbitFuzzerTestFunction(arg1, arg2)
#else
#define ORBIT_DEFINE_FUZZER(arg1, arg2)                             \
  static int OrbitFuzzerTestFunction(arg1, arg2);                   \
  extern "C" int LLVMFuzzerTestOneInput(uint8_t* buf, size_t len) { \
    return OrbitFuzzerTestFunction(buf, len);                       \
  }                                                                 \
  int OrbitFuzzerTestFunction(arg1, arg2)
#endif

#endif  // FUZZING_UTILS_FUZZER_H_
