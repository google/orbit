//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <sstream>

#include "Utils.h"
#include "absl/strings/str_format.h"

#if __linux__
#define FUNCTION_NAME __PRETTY_FUNCTION__
#else
#define FUNCTION_NAME __FUNCTION__
#endif

#define PRINT PrintDbg
#define PRINT_VAR(var) PrintVar(#var, var)
#define PRINT_VAR_INL(var) PrintVar(#var, var, true)
#define PRINT_FUNC PrintFunc(FUNCTION_NAME, __FILE__, __LINE__)
#define VAR_TO_STR(var) VarToStr(#var, var)
#define VAR_TO_CHAR(var) VarToStr(#var, var).c_str()
#define VAR_TO_ANSI(var) VarToAnsi(#var, var).c_str()

#define FATAL(...) { \
  PRINT("FATAL: "); \
  PRINT_FUNC; \
  PRINT(__VA_ARGS__); \
  PRINT("\n"); \
  abort(); \
}

#define CHECK(assertion) {\
  if (__builtin_expect(!(assertion), false)) {\
    FATAL(#assertion);\
  }\
}

//-----------------------------------------------------------------------------
inline void PrintVar(const char* a_VarName, const std::wstring& a_Value,
                     bool a_SameLine = false) {
  OutputDebugStringA(a_VarName);
  std::wstring value = std::wstring(L" = ") + a_Value;
  OutputDebugStringW(value.c_str());
  if (!a_SameLine) {
    OutputDebugStringA("\r\n");
  }
}

//-----------------------------------------------------------------------------
template <class T>
inline void PrintVar(const char* a_VarName, const T& a_Value,
                     bool a_SameLine = false) {
  std::stringstream l_StringStream;
  l_StringStream << a_VarName << " = " << a_Value;
  if (!a_SameLine) l_StringStream << std::endl;
  OutputDebugStringA(l_StringStream.str().c_str());
}

//-----------------------------------------------------------------------------
inline void PrintVar(const char* a_VarName, const wchar_t* a_Value,
                     bool a_SameLine = false) {
  std::wstring value(a_Value);
  PrintVar(a_VarName, value, a_SameLine);
}

//-----------------------------------------------------------------------------
template <class T>
inline std::wstring VarToStr(const char* a_VarName, const T& a_Value) {
  std::stringstream l_StringStream;
  l_StringStream << a_VarName << " = " << a_Value << std::endl;
  return s2ws(l_StringStream.str());
}

//-----------------------------------------------------------------------------
template <class T>
inline std::string VarToAnsi(const char* a_VarName, const T& a_Value) {
  std::stringstream l_StringStream;
  l_StringStream << a_VarName << " = " << a_Value;
  return l_StringStream.str();
}

//-----------------------------------------------------------------------------
inline void PrintFunc(const char* function, const char* file, int line) {
  std::string func = absl::StrFormat("%s %s(%i) TID: %u\n", function, file,
                                     line, GetCurrentThreadId());
  OutputDebugStringA(func.c_str());
}

//-----------------------------------------------------------------------------
inline void PrintDbg(const char* msg, ...) {
  va_list ap;
  const int BUFF_SIZE = 4096;
  char text[BUFF_SIZE] = {
      0,
  };
  va_start(ap, msg);
  vsnprintf_s(text, BUFF_SIZE - 1, msg, ap);
  va_end(ap);

  OutputDebugStringA(text);
}

//-----------------------------------------------------------------------------
inline void PrintDbg(const WCHAR* msg, ...) {
  va_list ap;
  const int BUFF_SIZE = 4096;
  WCHAR text[BUFF_SIZE] = {
      0,
  };
  va_start(ap, msg);
  _vsnwprintf_s(text, BUFF_SIZE - 1, msg, ap);
  va_end(ap);
  OutputDebugStringW(text);
}

//-----------------------------------------------------------------------------
inline void PrintDbg(const std::string& a_Msg) {
  OutputDebugStringA(a_Msg.c_str());
}

//-----------------------------------------------------------------------------
inline void PrintDbg(const std::wstring& a_Msg) {
  OutputDebugStringW(a_Msg.c_str());
}

//-----------------------------------------------------------------------------
inline void PrintLastError() {
  PrintVar("Last error: ", GetLastErrorAsString());
}
