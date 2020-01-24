//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <autoresetevent.h>
#include <condition_variable>
#include <mutex>
#include <thread>

// Moodycamel's concurrent queue
#ifdef WIN32
#pragma warning(push, 0)
#endif
#include <concurrentqueue.h>
#ifdef WIN32
#pragma warning(pop)
#endif

// HEA-L's oqpi
#ifdef _WIN32
#define OQPI_USE_DEFAULT
#include "../external/oqpi/include/oqpi.hpp"
using oqpi_tk = oqpi::default_helpers;
#endif

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

//-----------------------------------------------------------------------------
inline void SetThreadName(HANDLE a_Thread, const wchar_t* a_ThreadName) {
  SetThreadDescription(a_Thread, a_ThreadName);
}

//-----------------------------------------------------------------------------
inline void SetCurrentThreadName(const wchar_t* a_ThreadName) {
  SetThreadDescription(GetCurrentThread(), a_ThreadName);
}

//-----------------------------------------------------------------------------
inline std::wstring GetThreadName(HANDLE a_Thread) {
  std::wstring name;
  PWSTR data = nullptr;
  HRESULT hr = GetThreadDescription(a_Thread, &data);

  if (SUCCEEDED(hr)) {
    name = data;
    LocalFree(data);
  }

  return name;
}

//-----------------------------------------------------------------------------
inline std::wstring GetCurrentThreadName() {
  return GetThreadName(GetCurrentThread());
}

#endif
