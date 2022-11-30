// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_GET_PROC_ADDRESS_H_
#define ORBIT_BASE_GET_PROC_ADDRESS_H_

#ifdef WIN32

#include <string>

#include "Logging.h"
#include "Result.h"

namespace orbit_base {

// Returns the address of a function in the specified loaded module.
ErrorMessageOr<void*> GetProcAddress(std::string_view module, std::string_view function);

// Utility function to cast the return of GetProcAddress into the specified function pointer type.
template <typename FunctionPrototypeT>
inline FunctionPrototypeT GetProcAddress(std::string_view module, std::string_view function) {
  auto result = GetProcAddress(module, function);
  if (result.has_error()) {
    ORBIT_ERROR("Calling GetProcAddress: %s", result.error().message());
  }
  return reinterpret_cast<FunctionPrototypeT>(result.value());
}

}  // namespace orbit_base

#endif  // WIN32

#endif  // ORBIT_BASE_GET_PROC_ADDRESS_H_
