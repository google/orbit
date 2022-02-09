// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// clang-format off
#include "OrbitBase/GetProcAddress.h"

#include <Windows.h>
#include <libloaderapi.h>
// clang-format on

#include <absl/strings/str_format.h>

#include "OrbitBase/GetLastError.h"

namespace orbit_base {

ErrorMessageOr<void*> GetProcAddress(const std::string& module, const std::string& function) {
  HMODULE module_handle = GetModuleHandleA(module.c_str());
  if (module_handle == nullptr) {
    return ErrorMessage(
        absl::StrFormat("Could not find module \"%s\" while looking for function \"%s\": %s",
                        module, function, GetLastErrorAsString()));
  }

  void* address = ::GetProcAddress(module_handle, function.c_str());
  if (address == nullptr) {
    return ErrorMessage(absl::StrFormat("Could not find function \"%s\" in module \"%s\": %s",
                                        function, module, GetLastErrorAsString()));
  }

  return address;
}

}  // namespace orbit_base
