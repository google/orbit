// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_RESULT_H_
#define ORBIT_BASE_RESULT_H_

#include <string>

#include "outcome.hpp"

template <typename T, typename E>
using Result = outcome::result<T, E, outcome::policy::terminate>;

class ErrorMessage final {
 public:
  ErrorMessage() = default;
  explicit ErrorMessage(std::string message) : message_(std::move(message)) {}

  [[nodiscard]] const std::string& message() const { return message_; }

 private:
  std::string message_;
};

#endif  // ORBIT_BASE_RESULT_H_
