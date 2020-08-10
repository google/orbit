// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include <autoresetevent.h>

#include <condition_variable>
#include <mutex>
#include <thread>

// Moodycamel's concurrent queue
#ifdef _WIN32
#pragma warning(push, 0)
#endif
#include <concurrentqueue.h>
#ifdef _WIN32
#pragma warning(pop)

// HEA-L's oqpi
#include <processthreadsapi.h>
#define OQPI_USE_DEFAULT
#include "oqpi.hpp"
using oqpi_tk = oqpi::default_helpers;
#include <Windows.h>
#endif

#include "Utils.h"

// Typedefs
typedef std::recursive_mutex Mutex;
typedef std::lock_guard<std::recursive_mutex> ScopeLock;
typedef std::unique_lock<std::recursive_mutex> UniqueLock;
typedef std::condition_variable ConditionVariable;
template <typename T>
using LockFreeQueue = moodycamel::ConcurrentQueue<T>;

#ifdef _WIN32
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO {
  DWORD dwType;      // Must be 0x1000.
  LPCSTR szName;     // Pointer to name (in user addr space).
  DWORD dwThreadID;  // Thread ID (-1=caller thread).
  DWORD dwFlags;     // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

inline void SetThreadNameFallback(HANDLE thread,
                                  const std::string& threadName) {
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = threadName.c_str();
  info.dwThreadID = GetThreadId(thread);
  info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable : 6320 6322)
  __try {
    RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR),
                   (ULONG_PTR*)&info);
  } __except (EXCEPTION_EXECUTE_HANDLER) {
  }
#pragma warning(pop)
}

inline void SetThreadName(HANDLE thread, const wchar_t* threadName) {
  using SetThreadDescriptionPtr = HRESULT(WINAPI*)(HANDLE, PCWSTR);
  static auto const setThreadDescriptionPtr = []() -> SetThreadDescriptionPtr {
    HMODULE kernel32 = LoadLibraryA("kernel32.dll");

    SetThreadDescriptionPtr ptr = nullptr;
    if (kernel32 != NULL) {
      return reinterpret_cast<SetThreadDescriptionPtr>(
          GetProcAddress(kernel32, "SetThreadDescription"));
    } else {
      return nullptr;
    }
  }();

  if (setThreadDescriptionPtr != nullptr) {
    (*setThreadDescriptionPtr)(thread, threadName);
  } else {
    SetThreadNameFallback(thread, ws2s(threadName));
  }
}

inline void SetCurrentThreadName(const wchar_t* threadName) {
  SetThreadName(GetCurrentThread(), threadName);
}

inline std::string GetThreadName(HANDLE thread) {
  std::string name;

  using GetThreadDescriptionPtr = HRESULT(WINAPI*)(HANDLE, PWSTR*);
  static auto const getThreadDescriptionPtr = []() -> GetThreadDescriptionPtr {
    HMODULE kernel32 = LoadLibraryA("kernel32.dll");

    GetThreadDescriptionPtr ptr = nullptr;
    if (kernel32 != NULL) {
      return reinterpret_cast<GetThreadDescriptionPtr>(
          GetProcAddress(kernel32, "GetThreadDescription"));
    } else {
      return nullptr;
    }
  }();

  if (getThreadDescriptionPtr != nullptr) {
    PWSTR data = nullptr;
    HRESULT hr = (*getThreadDescriptionPtr)(thread, &data);

    if (SUCCEEDED(hr)) {
      name = ws2s(data);
      LocalFree(data);
    }
  }

  return name;
}

inline std::string GetCurrentThreadName() {
  return GetThreadName(GetCurrentThread());
}

#endif

#ifdef __linux__
#include <sys/syscall.h>
#include <unistd.h>

inline pid_t GetCurrentThreadId() {
  thread_local pid_t current_tid = syscall(__NR_gettid);
  return current_tid;
}
#endif
