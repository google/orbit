#ifndef ORBIT_CORE_PRINT_VAR_H_
#define ORBIT_CORE_PRINT_VAR_H_

#include <sstream>

#include "OrbitBase/Logging.h"
#include "Utils.h"
#include "absl/strings/str_format.h"

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

#endif  // ORBIT_CORE_PRINT_VAR_H_
