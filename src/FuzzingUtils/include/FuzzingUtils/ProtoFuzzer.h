// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FUZZING_UTILS_PROTO_FUZZER_H_
#define FUZZING_UTILS_PROTO_FUZZER_H_

#include <libfuzzer/libfuzzer_macro.h>

#include <functional>

#include "OrbitBase/Logging.h"

// Use the macro `ORBIT_PROTO_DEFINE_FUZZER` to define an LLVM-based fuzzer that automatically
// catches exceptions thrown by `CHECK`, `UNREACHABLE`, and other in non-fuzzing-mode aborting
// macros. The function takes a const-ref to a protobuf message type (a class inheriting from
// `protobuf::Message`).
//
// Example:
// ORBIT_DEFINE_PROTO_FUZZER(const WhateverProto& proto) {
//  // Put your fuzzing code here.
// }

#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
#define ORBIT_DEFINE_PROTO_FUZZER(arg)                                               \
  static void OrbitFuzzerTestFunction(arg);                                          \
  using FuzzerProtoType = std::remove_const<std::remove_reference<                   \
      std::function<decltype(OrbitFuzzerTestFunction)>::argument_type>::type>::type; \
  DEFINE_PROTO_FUZZER(const FuzzerProtoType& proto) {                                \
    try {                                                                            \
      OrbitFuzzerTestFunction(proto);                                                \
    } catch (const orbit_base::FuzzingException&) {                                  \
    }                                                                                \
  }                                                                                  \
  void OrbitFuzzerTestFunction(arg)
#else
#define ORBIT_DEFINE_PROTO_FUZZER(arg) DEFINE_PROTO_FUZZER(arg)
#endif

#endif  // FUZZING_UTILS_PROTO_FUZZER_H_
