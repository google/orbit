// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_PRINT_VAR_H_
#define ORBIT_CORE_PRINT_VAR_H_

#include <sstream>

#define VAR_TO_STR(var) VariableToString(#var, var)

template <class T>
inline std::string VariableToString(const char* name, const T& value) {
  std::stringstream string_stream;
  string_stream << name << " = " << value;
  return string_stream.str();
}

#endif  // ORBIT_CORE_PRINT_VAR_H_
