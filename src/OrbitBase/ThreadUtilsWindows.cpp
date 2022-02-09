// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <Windows.h>
#include <processthreadsapi.h>

#include <string>

#include "OrbitBase/GetProcAddress.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"

namespace orbit_base {

static constexpr uint32_t kInvalidWindowsThreadId = 0;
static constexpr uint32_t kInvalidWindowsProcessId_0 = 0;
static constexpr uint32_t kInvalidWindowsProcessId_1 = 0xffffffff;

// On Windows, thread and process ids are observed to be multiples of 4. Even though there is no
// formal guarantee for this property, the current implementation of cross platform thread process
// ids rely on it. https://devblogs.microsoft.com/oldnewthing/20080228-00/?p=23283
static inline [[nodiscard]] bool IsMultipleOfFour(uint32_t value) { return (value & 3) == 0; }

uint32_t GetCurrentThreadId() { return FromNativeThreadId(GetCurrentThreadIdNative()); }

uint32_t GetCurrentProcessId() { return FromNativeProcessId(GetCurrentProcessIdNative()); }

bool IsValidThreadId(uint32_t tid) {
  return tid != orbit_base::kInvalidThreadId && IsMultipleOfFour(tid);
}

bool IsValidProcessId(uint32_t pid) {
  return pid != orbit_base::kInvalidProcessId && IsMultipleOfFour(pid);
}

uint32_t GetCurrentThreadIdNative() {
  thread_local uint32_t current_tid = ::GetCurrentThreadId();
  return current_tid;
}

uint32_t GetCurrentProcessIdNative() { return ::GetCurrentProcessId(); }

uint32_t FromNativeThreadId(uint32_t tid) {
  ORBIT_CHECK(IsMultipleOfFour(tid) || tid == kInvalidWindowsThreadId);
  return tid != kInvalidWindowsThreadId ? tid : orbit_base::kInvalidThreadId;
}

uint32_t FromNativeProcessId(uint32_t pid) {
  bool is_invalid_pid = (pid == kInvalidWindowsProcessId_0 || pid == kInvalidWindowsProcessId_1);
  ORBIT_CHECK(IsMultipleOfFour(pid) || is_invalid_pid);
  return !is_invalid_pid ? pid : orbit_base::kInvalidProcessId;
}

uint32_t ToNativeThreadId(uint32_t tid) {
  ORBIT_CHECK(IsMultipleOfFour(tid) || tid == orbit_base::kInvalidThreadId);
  return tid != orbit_base::kInvalidThreadId ? tid : kInvalidWindowsThreadId;
}

uint32_t ToNativeProcessId(uint32_t pid) {
  ORBIT_CHECK(IsMultipleOfFour(pid) || pid == orbit_base::kInvalidProcessId);
  return pid != orbit_base::kInvalidProcessId ? pid : kInvalidWindowsProcessId_0;
}

std::string GetThreadName(uint32_t tid) { return GetThreadNameNative(ToNativeThreadId(tid)); }

std::string GetThreadNameNative(uint32_t tid) {
  static const std::string kEmptyString;

  // Thread 0 is the "System" thread.
  if (tid == 0) return "System";

  // Find "GetThreadDescription" procedure.
  static auto get_thread_description = orbit_base::GetProcAddress<HRESULT(WINAPI*)(HANDLE, PWSTR*)>(
      "kernel32.dll", "GetThreadDescription");
  if (get_thread_description == nullptr) {
    ORBIT_ERROR("Getting thread name from id %u with proc[%llx]", tid, get_thread_description);
    return kEmptyString;
  }

  // Get thread handle from tid.
  HANDLE thread_handle = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, tid);
  if (thread_handle == nullptr) {
    ORBIT_ERROR("Retrieving thread handle for tid %u", tid);
    return kEmptyString;
  }

  // Get thread name from handle.
  PWSTR thread_name_pwstr;
  if (SUCCEEDED((*get_thread_description)(thread_handle, &thread_name_pwstr))) {
    std::wstring thread_name_w(thread_name_pwstr);
    LocalFree(thread_name_pwstr);
    std::string thread_name(thread_name_w.begin(), thread_name_w.end());
    return thread_name;
  }

  ORBIT_ERROR("Getting thread name from id %u with proc[%llx]", tid, get_thread_description);
  return kEmptyString;
}

void SetCurrentThreadName(const char* name) {
  static auto set_thread_description =
      GetProcAddress<HRESULT(WINAPI*)(HANDLE, PCWSTR)>("kernel32.dll", "SetThreadDescription");
  std::wstring wide_name(name, name + strlen(name));
  if (set_thread_description == nullptr ||
      !SUCCEEDED((*set_thread_description)(GetCurrentThread(), wide_name.c_str()))) {
    ORBIT_ERROR("Setting thread name %s with proc[%llx]", name, set_thread_description);
  }
}

}  // namespace orbit_base
