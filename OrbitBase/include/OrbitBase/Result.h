// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_RESULT_H_
#define ORBIT_BASE_RESULT_H_

#include <string_view>

#include "outcome.hpp"

template <typename T, typename E>
using Result = outcome::result<T, E, outcome::policy::terminate>;

class ErrorMessage final {
 public:
  ErrorMessage() = default;
  explicit ErrorMessage(std::string_view message)
      : message_(message.data(), message.size()) {}

  ErrorMessage(const ErrorMessage&) = default;
  ErrorMessage& operator=(const ErrorMessage&) = default;
  ErrorMessage(ErrorMessage&&) = default;
  ErrorMessage& operator=(ErrorMessage&&) = default;

  [[nodiscard]] const std::string& message() const { return message_; }

 private:
  std::string message_;
};

#endif  // ORBIT_BASE_RESULT_H_
