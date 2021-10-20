// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_HANDLE_CLOSER_H_
#define WINDOWS_UTILS_HANDLE_CLOSER_H_

#include <windows.h>

#include "OrbitBase/UniqueResource.h"

namespace orbit_windows_utils {

auto CreateHandleCloser(HANDLE handle) {
  return orbit_base::unique_resource(handle, [](HANDLE h) {
    if (h != nullptr) {
      CloseHandle(h);
    }
  });
}

}  // namespace orbit_windows_utils

#endif
