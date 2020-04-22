//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <sstream>

#include "absl/strings/str_format.h"
#include "OrbitBase/Logging.h"
#include "Utils.h"

#define PRINT_VAR(var) LOG("%s", VAR_TO_STR(var).c_str())
#define PRINT_FUNC LOG("%s tid:%u", FUNCTION_NAME, GetCurrentThreadId())
#define VAR_TO_STR(var) VariableToString(#var, var)

//-----------------------------------------------------------------------------
template <class T>
inline std::string VariableToString(const char* name, const T& value) {
  std::stringstream string_stream;
  string_stream << name << " = " << value;
  return string_stream.str();
}

//-----------------------------------------------------------------------------
inline void PrintLastError() { PRINT_VAR(GetLastErrorAsString()); }
