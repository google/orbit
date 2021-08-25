// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_CONTEXT_H_
#define ORBIT_SSH_CONTEXT_H_

#include "OrbitBase/Result.h"

namespace orbit_ssh {

class Context {
 public:
  static outcome::result<Context> Create();

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  Context(Context&&);
  Context& operator=(Context&&);

  ~Context();

  [[nodiscard]] bool active() const { return active_; }

 private:
  explicit Context() = default;
  bool active_ = true;
};

}  // namespace orbit_ssh
#endif  // ORBIT_SSH_CONTEXT_H_
