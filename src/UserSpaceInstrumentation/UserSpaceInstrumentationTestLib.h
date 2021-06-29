// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_TEST_LIB_H_
#define USER_SPACE_INSTRUMENTATION_TEST_LIB_H_

#include <cstdint>

// This library is merely used in tests: The tests inject a binary produced by this code into its
// child and use the functions defined here.

// Returns 42.
extern "C" int TrivialFunction();

// Returns sum of the parameters.
extern "C" uint64_t TrivialSum(uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4,
                               uint64_t p5);

// Payload called on entry of an instrumented function. Needs to record the return address of the
// function (in order to have it available in `ExitPayload`). `function_address` is the address of
// the instrumented function.
extern "C" void EntryPayload(uint64_t return_address, uint64_t function_address);

// Payload called on exit of an instrumented function. Needs to return the actual return address of
// the function such that the execution can be continued there.
extern "C" uint64_t ExitPayload();

// Overwrites rdi, rsi, rdx, rcx, r8, r9, rax, r10. These registers are used to hand over parameters
// to a called function. This function is used to assert our backup of these registers works
// properly. The two functions below do the same thing for SSE/AVX registers that can be used to
// hand over floating point parameters.
extern "C" void EntryPayloadClobberParameterRegisters(uint64_t return_address,
                                                      uint64_t function_address);
extern "C" void EntryPayloadClobberXmmRegisters(uint64_t return_address, uint64_t function_address);
extern "C" void EntryPayloadClobberYmmRegisters(uint64_t return_address, uint64_t function_address);

#endif  // USER_SPACE_INSTRUMENTATION_TEST_LIB_H_