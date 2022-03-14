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

// Also returns the sum of the parameters, but it uses the Microsoft x64 calling convention.
extern "C" __attribute__((ms_abi)) uint64_t TrivialSumWithMsAbi(uint64_t p0, uint64_t p1,
                                                                uint64_t p2, uint64_t p3);

// Payload called on entry of an instrumented function. Needs to record the return address of the
// function (in order to have it available in `ExitPayload`) and the stack pointer (i.e., the
// address of the return address). `function_id` is the id of the instrumented function. Also needs
// to overwrite the return address stored at `stack_pointer` with the `return_trampoline_address`.
extern "C" void EntryPayload(uint64_t return_address, uint64_t function_id, uint64_t stack_pointer,
                             uint64_t return_trampoline_address);

// Payload called on exit of an instrumented function. Needs to return the actual return address of
// the function such that the execution can be continued there.
extern "C" uint64_t ExitPayload();

// Performs a MOVAPS from an address at a distance multiple of 16 from RBP. As the 128-bit memory
// operands must be 16-byte aligned (SIGSEGV is raised otherwise), this verifies that the stack was
// aligned to 16 bytes before calling this EntryPayload.
// We are assuming that this function updates the frame pointer, i.e., that it starts with
// `push rbp; mov rbp, rsp`.
extern "C" void EntryPayloadAlignedCopy(uint64_t return_address, uint64_t function_id,
                                        uint64_t stack_pointer, uint64_t return_trampoline_address);

// Overwrites rdi, rsi, rdx, rcx, r8, r9, rax, r10. These registers are used to hand over parameters
// to a called function. This function is used to assert our backup of these registers works
// properly. The two functions below do the same thing for SSE/AVX registers that can be used to
// hand over floating point parameters.
extern "C" void EntryPayloadClobberParameterRegisters(uint64_t return_address, uint64_t function_id,
                                                      uint64_t stack_pointer,
                                                      uint64_t return_trampoline_address);
extern "C" void EntryPayloadClobberXmmRegisters(uint64_t return_address, uint64_t function_id,
                                                uint64_t stack_pointer,
                                                uint64_t return_trampoline_address);
extern "C" void EntryPayloadClobberYmmRegisters(uint64_t return_address, uint64_t function_id,
                                                uint64_t stack_pointer,
                                                uint64_t return_trampoline_address);

#endif  // USER_SPACE_INSTRUMENTATION_TEST_LIB_H_