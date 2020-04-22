//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <cstdint>

#ifdef _WIN32
#include <BaseTsd.h>
#include <tchar.h>
#include <wtypes.h>
#endif

#ifdef __linux__
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <wchar.h>

#include <iostream>

#define wcstok_s wcstok
#define WCHAR wchar_t
#define GUID unsigned long long
#define ULONG64 unsigned long long
#define DWORD64 ULONG64
typedef unsigned long DWORD;
typedef DWORD64 IntervalType;
typedef DWORD64 EpochType;
typedef int64_t __int64;
#define TCHAR wchar_t
#define MAX_PATH PATH_MAX
#define HANDLE void*
#define __int8 char
#define LONG long
typedef DWORD ULONG;
typedef unsigned char byte;
// struct TypeInfo{};
#define HMODULE void*
inline void SetCurrentThreadName(const wchar_t*) {}
#define FILETIME ULONG64
#define FLT_MAX __FLT_MAX__
#define TEXT(x) L##x
#define USHORT unsigned short
#define UCHAR unsigned char
inline void Sleep(int millis) { usleep(millis * 1000); }

inline pid_t GetCurrentThreadId() {
  thread_local pid_t current_tid = syscall(__NR_gettid);
  return current_tid;
}

typedef struct _M128A {
  uint64_t Low;
  uint64_t High;
} M128A;

#define vsnprintf_s vsnprintf
#define _vsnwprintf_s vswprintf

#endif
