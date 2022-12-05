// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "RegisterState.h"

#include <absl/strings/str_format.h>
#include <cpuid.h>
#include <elf.h>
#include <errno.h>
#include <stdint.h>
#include <sys/ptrace.h>
#include <sys/uio.h>

#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitBase/SafeStrerror.h"

namespace orbit_user_space_instrumentation {

namespace {

// Some notes reguarding the calls to cpuid below:
// Cpuid can be used to query all sorts of information about the cpu (presence of features,
// specifications, ...). Cpuid takes one parameter in eax. In Intel's terminology, this is called
// the Cpuid "leaf". Some leaves have "sub-leafs" i.e. they take a second paramter in ecx. The
// sub-leaf is sometimes called "count". The return values end up in registers eax, ... , edx. The
// wrappers from cpuid.h simplify error and parameter handling. cpuid.h also has some defines and
// useful comments to figure out what can be queried. More comprehensive info:
// https://www.sandpile.org/x86/cpuid.htm

// Return the size of the XSave area on this cpu.
[[nodiscard]] ErrorMessageOr<size_t> GetXSaveAreaSize() {
  uint32_t eax = 0;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;
  if (!__get_cpuid(0x01, &eax, &ebx, &ecx, &edx) || !(ecx & bit_XSAVE)) {
    return ErrorMessage("XSAVE is not supported by the Cpu.");
  }
  if (!__get_cpuid_count(0x0d, 0x00, &eax, &ebx, &ecx, &edx)) {
    return ErrorMessage("XSAVE is not supported by the Cpu.");
  }
  return static_cast<size_t>(ecx);
}

// Return offset of the YMMx registers inside the extended section of the XSave area.
[[nodiscard]] ErrorMessageOr<size_t> GetAvxOffset() {
  uint32_t eax = 0;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;
  if (!__get_cpuid(0x01, &eax, &ebx, &ecx, &edx) || !(ecx & bit_AVX)) {
    return ErrorMessage("AVX is not supported by the Cpu.");
  }
  if (!__get_cpuid_count(0x0d, 0x02, &eax, &ebx, &ecx, &edx)) {
    return ErrorMessage("AVX offset query failed.");
  }
  return static_cast<size_t>(ebx);
}

}  // namespace

bool RegisterState::Hasx87DataStored() {
  return (static_cast<uint64_t>(GetXSaveHeader()->xstate_bv) &
          static_cast<uint64_t>(XSaveHeader::StateComponents::kX87)) != 0;
}

bool RegisterState::HasSseDataStored() {
  return (static_cast<uint64_t>(GetXSaveHeader()->xstate_bv) &
          static_cast<uint64_t>(XSaveHeader::StateComponents::kSse)) != 0;
}

bool RegisterState::HasAvxDataStored() {
  return (static_cast<uint64_t>(GetXSaveHeader()->xstate_bv) &
          static_cast<uint64_t>(XSaveHeader::StateComponents::kAvx)) != 0;
}

ErrorMessageOr<void> RegisterState::BackupRegisters(pid_t tid) {
  tid_ = tid;

  iovec iov;
  iov.iov_base = &general_purpose_registers_;
  iov.iov_len = sizeof(GeneralPurposeRegisters);
  auto result = ptrace(PTRACE_GETREGSET, tid, NT_PRSTATUS, &iov);
  if (result == -1) {
    return ErrorMessage(absl::StrFormat("PTRACE_GETREGS, NT_PRSTATUS failed with errno: %d: %s",
                                        errno, SafeStrerror(errno)));
  }

  if (iov.iov_len == sizeof(GeneralPurposeRegisters32)) {
    bitness_ = Bitness::k32Bit;
  } else if (iov.iov_len == sizeof(GeneralPurposeRegisters64)) {
    bitness_ = Bitness::k64Bit;
  } else {
    ORBIT_FATAL("Bitness is neither 32 or 64 bit.");
  }

  auto xsave_area_size = GetXSaveAreaSize();
  if (xsave_area_size.has_error()) {
    return xsave_area_size.error();
  }
  xsave_area_.resize(xsave_area_size.value());

  iov.iov_len = xsave_area_.size();
  iov.iov_base = xsave_area_.data();
  result = ptrace(PTRACE_GETREGSET, tid, NT_X86_XSTATE, &iov);
  if (result == -1) {
    return ErrorMessage(absl::StrFormat("PTRACE_GETREGS, NT_X86_XSTATE failed with errno: %d: %s",
                                        errno, SafeStrerror(errno)));
  }

  auto avx_offset = GetAvxOffset();
  if (!avx_offset.has_error()) {
    avx_offset_ = avx_offset.value();
  }

  return outcome::success();
}

ErrorMessageOr<void> RegisterState::RestoreRegisters() {
  // BackupRegisters needs to be called before RestoreRegisters.
  ORBIT_CHECK(tid_ != -1);

  iovec iov;
  iov.iov_base = &general_purpose_registers_;
  iov.iov_len = (bitness_ == Bitness::k32Bit) ? sizeof(GeneralPurposeRegisters32)
                                              : sizeof(GeneralPurposeRegisters64);
  auto result = ptrace(PTRACE_SETREGSET, tid_, NT_PRSTATUS, &iov);
  if (result == -1) {
    return ErrorMessage(
        absl::StrFormat("PTRACE_SETREGSET failed to write NT_PRSTATUS with errno: %d: %s", errno,
                        SafeStrerror(errno)));
  }

  iov.iov_len = xsave_area_.size();
  iov.iov_base = xsave_area_.data();
  result = ptrace(PTRACE_SETREGSET, tid_, NT_X86_XSTATE, &iov);
  if (result == -1) {
    return ErrorMessage(
        absl::StrFormat("PTRACE_SETREGSET failed to write NT_X86_XSTATE with errno: %d: %s", errno,
                        SafeStrerror(errno)));
  }
  return outcome::success();
}

}  // namespace orbit_user_space_instrumentation