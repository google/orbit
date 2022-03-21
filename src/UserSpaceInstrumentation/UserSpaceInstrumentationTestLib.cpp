// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UserSpaceInstrumentationTestLib.h"

#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <ratio>
#include <stack>

namespace {

struct ReturnAddressOfFunction {
  ReturnAddressOfFunction(uint64_t return_address, uint64_t function_id, uint64_t stack_pointer)
      : return_address(return_address), function_id(function_id), stack_pointer(stack_pointer) {}
  uint64_t return_address;
  uint64_t function_id;
  uint64_t stack_pointer;
};

thread_local std::stack<ReturnAddressOfFunction> return_addresses;

}  // namespace

int TrivialFunction() { return 42; }

uint64_t TrivialSum(uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5) {
  return p0 + p1 + p2 + p3 + p4 + p5;
}

uint64_t TrivialSumWithMsAbi(uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3) {
  return p0 + p1 + p2 + p3;
}

void EntryPayload(uint64_t return_address, uint64_t function_id, uint64_t stack_pointer,
                  uint64_t return_trampoline_address) {
  // Verify that the return address and the stack pointer (location of the return address) are
  // coherent. If not, the tests that use trampolines that call this EntryPayload will not succeed.
  if (*reinterpret_cast<uint64_t*>(stack_pointer) != return_address) {
    abort();
  }

  return_addresses.emplace(return_address, function_id, stack_pointer);

  *reinterpret_cast<uint64_t*>(stack_pointer) = return_trampoline_address;
}

uint64_t ExitPayload() {
  ReturnAddressOfFunction current_return_address = return_addresses.top();
  return_addresses.pop();

  using std::chrono::system_clock;
  constexpr std::chrono::duration<int, std::ratio<1, 1000000>> k500Microseconds(500);
  static system_clock::time_point last_logged_event = system_clock::now() - 2 * k500Microseconds;
  static uint64_t skipped = 0;
  // Rate limit log output to once every 500 microseconds.
  const system_clock::time_point now = system_clock::now();
  if (now - last_logged_event > k500Microseconds) {
    if (skipped > 0) {
      printf(" ( %lu skipped events )\n", skipped);
    }
    printf("Returned from function with id %lu\n", current_return_address.function_id);
    last_logged_event = now;
    skipped = 0;
  } else {
    skipped++;
  }

  return current_return_address.return_address;
}

void EntryPayloadAlignedCopy(uint64_t return_address, uint64_t function_id, uint64_t stack_pointer,
                             uint64_t return_trampoline_address) {
  if (*reinterpret_cast<uint64_t*>(stack_pointer) != return_address) abort();
  return_addresses.emplace(return_address, function_id, stack_pointer);
  *reinterpret_cast<uint64_t*>(stack_pointer) = return_trampoline_address;

  __asm__ __volatile__("movaps -0x10(%%rbp), %%xmm0\n\t" : : :);
}

// rdi, rsi, rdx, rcx, r8, r9, rax, r10
void EntryPayloadClobberParameterRegisters(uint64_t return_address, uint64_t function_id,
                                           uint64_t stack_pointer,
                                           uint64_t return_trampoline_address) {
  if (*reinterpret_cast<uint64_t*>(stack_pointer) != return_address) abort();
  return_addresses.emplace(return_address, function_id, stack_pointer);
  *reinterpret_cast<uint64_t*>(stack_pointer) = return_trampoline_address;

  __asm__ __volatile__(
      "mov $0xffffffffffffffff, %%rdi\n\t"
      "mov $0xffffffffffffffff, %%rsi\n\t"
      "mov $0xffffffffffffffff, %%rdx\n\t"
      "mov $0xffffffffffffffff, %%rcx\n\t"
      "mov $0xffffffffffffffff, %%r8\n\t"
      "mov $0xffffffffffffffff, %%r9\n\t"
      "mov $0xffffffffffffffff, %%rax\n\t"
      "mov $0xffffffffffffffff, %%r10\n\t"
      :
      :
      :);
}

void EntryPayloadClobberXmmRegisters(uint64_t return_address, uint64_t function_id,
                                     uint64_t stack_pointer, uint64_t return_trampoline_address) {
  if (*reinterpret_cast<uint64_t*>(stack_pointer) != return_address) abort();
  return_addresses.emplace(return_address, function_id, stack_pointer);
  *reinterpret_cast<uint64_t*>(stack_pointer) = return_trampoline_address;

  __asm__ __volatile__(
      "movdqu 0x3a(%%rip), %%xmm0\n\t"
      "movdqu 0x32(%%rip), %%xmm1\n\t"
      "movdqu 0x2a(%%rip), %%xmm2\n\t"
      "movdqu 0x22(%%rip), %%xmm3\n\t"
      "movdqu 0x1a(%%rip), %%xmm4\n\t"
      "movdqu 0x12(%%rip), %%xmm5\n\t"
      "movdqu 0x0a(%%rip), %%xmm6\n\t"
      "movdqu 0x02(%%rip), %%xmm7\n\t"
      "jmp label_clobber_xmm_after_data\n\t"
      ".quad 0xffffffffffffffff, 0xffffffffffffffff \n\t"
      "label_clobber_xmm_after_data:\n\t"
      :
      :
      :);
}

void EntryPayloadClobberYmmRegisters(uint64_t return_address, uint64_t function_id,
                                     uint64_t stack_pointer, uint64_t return_trampoline_address) {
  if (*reinterpret_cast<uint64_t*>(stack_pointer) != return_address) abort();
  return_addresses.emplace(return_address, function_id, stack_pointer);
  *reinterpret_cast<uint64_t*>(stack_pointer) = return_trampoline_address;

  __asm__ __volatile__(
      "vmovdqu 0x3a(%%rip), %%ymm0\n\t"
      "vmovdqu 0x32(%%rip), %%ymm1\n\t"
      "vmovdqu 0x2a(%%rip), %%ymm2\n\t"
      "vmovdqu 0x22(%%rip), %%ymm3\n\t"
      "vmovdqu 0x1a(%%rip), %%ymm4\n\t"
      "vmovdqu 0x12(%%rip), %%ymm5\n\t"
      "vmovdqu 0x0a(%%rip), %%ymm6\n\t"
      "vmovdqu 0x02(%%rip), %%ymm7\n\t"
      "jmp label_clobber_ymm_after_data\n\t"
      ".quad 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff \n\t"
      "label_clobber_ymm_after_data:\n\t"
      :
      :
      :);
}
