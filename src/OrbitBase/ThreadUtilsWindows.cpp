// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <Windows.h>
#include <processthreadsapi.h>

#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"

namespace orbit_base {

static constexpr uint32_t kInvalidWindowsThreadId = 0;
static constexpr uint32_t kInvalidWindowsProcessId_0 = 0;
static constexpr uint32_t kInvalidWindowsProcessId_1 = 0xffffffff;

static [[nodiscard]] bool IsInvalidWindowsThreadId(uint32_t tid) {
  return tid == kInvalidWindowsThreadId;
}

static [[nodiscard]] bool IsInvalidWindowsProcessId(uint32_t pid) {
  return pid == kInvalidWindowsProcessId_0 || pid == kInvalidWindowsProcessId_1;
}

uint32_t GetCurrentThreadId() { return GetThreadIdFromNative(GetCurrentThreadIdNative()); }

uint32_t GetCurrentProcessId() { return GetProcessIdFromNative(GetCurrentProcessIdNative()); }

uint32_t GetCurrentThreadIdNative() {
  thread_local uint32_t current_tid = ::GetCurrentThreadId();
  return current_tid;
}

uint32_t GetCurrentProcessIdNative() { return ::GetCurrentProcessId(); }

uint32_t GetThreadIdFromNative(uint32_t tid) {
  return IsInvalidWindowsThreadId(tid)) ? orbit_base::kInvalidThreadId : tid;
}

uint32_t GetProcessIdFromNative(uint32_t pid) {
  return IsInvalidWindowsProcessId(pid)) ? orbit_base::kInvalidProcessId : pid;
}

uint32_t GetNativeThreadId(uint32_t tid) {
  return tid == orbit_base::kInvalidThreadId) ? kInvalidWindowsThreadId : tid;
}

uint32_t GetNativeProcessId(uint32_t pid) {
  return pid == orbit_base::kInvalidProcessId ? kInvalidWindowsProcessId_0 : pid;
}

template <typename FunctionPrototypeT>
static FunctionPrototypeT GetProcAddress(const std::string& library, const std::string& procedure) {
  HMODULE module_handle = LoadLibraryA(library.c_str());
  if (module_handle == nullptr) {
    ERROR("Could not find procedure %s in %s", procedure, library);
    return nullptr;
  }
  return reinterpret_cast<FunctionPrototypeT>(::GetProcAddress(module_handle, procedure.c_str()));
}

std::string GetThreadNameNative(uint32_t tid) {
  static const std::string kEmptyString;

  // Find "GetThreadDescription" procedure.
  static auto get_thread_description =
      GetProcAddress<HRESULT(WINAPI*)(HANDLE, PWSTR*)>("kernel32.dll", "GetThreadDescription");
  if (get_thread_description == nullptr) {
    ERROR("Getting thread name from id %u with proc[%llx]", tid, get_thread_description);
    return kEmptyString;
  }

  // Get thread handle from tid.
  HANDLE thread_handle = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, tid);
  if (thread_handle == nullptr) {
    ERROR("Retrieving thread handle for tid %u", tid);
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

  ERROR("Getting thread name from id %u with proc[%llx]", tid, get_thread_description);
  return kEmptyString;
}

void SetCurrentThreadName(const char* name) {
  static auto set_thread_description =
      GetProcAddress<HRESULT(WINAPI*)(HANDLE, PCWSTR)>("kernel32.dll", "SetThreadDescription");
  std::wstring wide_name(name, name + strlen(name));
  if (set_thread_description == nullptr ||
      !SUCCEEDED((*set_thread_description)(GetCurrentThread(), wide_name.c_str()))) {
    ERROR("Setting thread name %s with proc[%llx]", name, set_thread_description);
  }
}

}  // namespace orbit_base
