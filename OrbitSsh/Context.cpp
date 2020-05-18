// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <OrbitSsh/Context.h>
#include <OrbitSsh/Error.h>
#include <libssh2.h>

namespace OrbitSsh {

outcome::result<Context> Context::Create() {
  const auto result = libssh2_init(0);

  if (result < 0) {
    return static_cast<Error>(result);
  }

  return Context{};
}

Context::Context(Context &&other)  noexcept: active_(other.active_) {
  other.active_ = false;
}

Context& Context::operator=(Context &&other)  noexcept{
  if (&other != this) {
    active_ = other.active_;
    other.active_ = false;
  }

  return *this;
}

Context::~Context() noexcept {
  if (active_) {
    libssh2_exit();
  }
}

} // namespace OrbitSsh
