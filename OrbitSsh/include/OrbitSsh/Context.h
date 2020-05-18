// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_CONTEXT_H_
#define ORBIT_SSH_CONTEXT_H_

#include <outcome.hpp>

namespace OrbitSsh {

class Context {
 public:
  static outcome::result<Context> Create();

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  Context(Context&&) noexcept;
  Context& operator=(Context&&) noexcept;

  ~Context() noexcept;

  bool isActiveContext() const { return active_; }

 private:
  explicit Context() = default;
  bool active_ = true;
};

}  // namespace OrbitSsh
#endif  // ORBIT_SSH_CONTEXT_H_
