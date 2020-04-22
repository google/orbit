//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <sstream>

#include "absl/strings/str_format.h"
#include "OrbitBase/Logging.h"
#include "Utils.h"

#if __linux__
#define FUNCTION_NAME __PRETTY_FUNCTION__
#else
#define FUNCTION_NAME __FUNCTION__
#endif

#define PRINT_VAR(var) PrintVar(#var, var)
#define PRINT_VAR_INL(var) PrintVar(#var, var, true)
#define PRINT_FUNC PrintFunc(FUNCTION_NAME, __FILE__, __LINE__)
#define VAR_TO_STR(var) VarToStr(#var, var)
#define VAR_TO_CHAR(var) VarToStr(#var, var).c_str()
#define VAR_TO_ANSI(var) VarToAnsi(#var, var).c_str()

//-----------------------------------------------------------------------------
template <class T>
inline void PrintVar(const char* name, const T& value, bool same_line = false) {
  std::stringstream stream;
  stream << name << " = " << value;
  if (!same_line) stream << std::endl;
  PRINT(stream.str().c_str());
}

//-----------------------------------------------------------------------------
inline void PrintVar(const char* name, const std::wstring& value_w,
                     bool same_line = false) {
  PrintVar(name, ws2s(value_w), same_line);
}

//-----------------------------------------------------------------------------
inline void PrintVar(const char* name, const wchar_t* value,
                     bool same_line = false) {
  PrintVar(name, ws2s(value), same_line);
}

//-----------------------------------------------------------------------------
template <class T>
inline std::wstring VarToStr(const char* name, const T& value) {
  std::stringstream string_stream;
  string_stream << name << " = " << value << std::endl;
  return s2ws(string_stream.str());
}

//-----------------------------------------------------------------------------
template <class T>
inline std::string VarToAnsi(const char* name, const T& value) {
  std::stringstream string_stream;
  string_stream << name << " = " << value;
  return string_stream.str();
}

//-----------------------------------------------------------------------------
inline void PrintFunc(const char* function, const char* file, int line) {
  std::string func = absl::StrFormat("%s %s(%i) TID: %u\n", function, file,
                                     line, GetCurrentThreadId());
  PRINT(func.c_str());
}

//-----------------------------------------------------------------------------
inline void PrintLastError() {
  PrintVar("Last error: ", GetLastErrorAsString());
}
