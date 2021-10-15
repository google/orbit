// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_HANDLE_CLOSER_H_
#define WINDOWS_UTILS_HANDLE_CLOSER_H_

#include <windows.h>

namespace orbit_windows_utils {

// Convenience class around HANDLE to make sure CloseHandle is called.
class HandleCloser {
 public:
  explicit HandleCloser(HANDLE handle) : handle_(handle) {}

  ~HandleCloser() { Close(); }

  void Close() {
    if (handle_ != nullptr) {
      CloseHandle(handle_);
      handle_ = nullptr;
    }
  }

  HandleCloser() = delete;
  HandleCloser(const HandleCloser& other) = delete;
  HandleCloser& operator=(const HandleCloser& other) = delete;
  HandleCloser(HandleCloser&& other) = delete;
  HandleCloser& operator=(HandleCloser&& other) = delete;

 private:
  HANDLE handle_ = nullptr;
};

}  // namespace orbit_windows_utils

#endif
