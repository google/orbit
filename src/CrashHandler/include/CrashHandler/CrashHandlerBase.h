// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CRASH_HANDLER_CRASH_HANDLER_BASE_H_
#define CRASH_HANDLER_CRASH_HANDLER_BASE_H_

namespace orbit_crash_handler {

class CrashHandlerBase {
 public:
  virtual void DumpWithoutCrash() const {};
  virtual void SetUploadsEnabled(bool){};
  virtual ~CrashHandlerBase() = default;
};

}  // namespace orbit_crash_handler

#endif  // CRASH_HANDLER_CRASH_HANDLER_BASE_H_
