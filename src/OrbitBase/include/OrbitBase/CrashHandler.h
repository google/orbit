// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_CRASH_HANDLER_BASE_H_
#define ORBIT_BASE_CRASH_HANDLER_BASE_H_

namespace orbit_base {

// orbit_base::CrashHandler is a stub, the real implementation is in
// orbit_crash_handler::CrashHandler.
class CrashHandler {
 public:
  virtual void DumpWithoutCrash() const {};
  virtual void SetUploadsEnabled(bool){};
  virtual ~CrashHandler() = default;
};

}  // namespace orbit_base

#endif  // ORBIT_BASE_CRASH_HANDLER_BASE_H_
