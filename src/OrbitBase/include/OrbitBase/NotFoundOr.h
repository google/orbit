// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_NOT_FOUND_OR_H_
#define ORBIT_BASE_NOT_FOUND_OR_H_

#include <string>
#include <utility>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

namespace orbit_base {

// NotFound type that carries a message
struct NotFound {
  explicit NotFound(std::string message = "") : message_(std::move(message)) {}

  [[nodiscard]] const std::string& message() const { return message_; }

 private:
  std::string message_;
};

// NotFoundOr can be used as the return type of a search operation, where the search can be
// unsuccessful and returns a message. Check whether a NotFoundOr object is
// a "not found" result with either `.has_error()` or `orbit_base::IsNotFound`.
// Get the message with `GetNotFoundMessage(const NotFoundOr<T>&)`.
template <typename T>
using NotFoundOr = Result<T, NotFound>;

// Free function to check whether a NotFoundOr type is "not found".
template <typename T>
[[nodiscard]] bool IsNotFound(const NotFoundOr<T>& not_found_or) {
  return not_found_or.has_error();
}

// Free function to get the not found message of a NotFoundOr object.
template <typename T>
[[nodiscard]] const std::string& GetNotFoundMessage(const NotFoundOr<T>& not_found_or) {
  ORBIT_CHECK(IsNotFound(not_found_or));
  return not_found_or.error().message();
}

// Free function to get the "found" content of a NotFoundOr object.
template <typename T>
[[nodiscard]] const T& GetFound(const NotFoundOr<T>& not_found_or) {
  ORBIT_CHECK(!IsNotFound(not_found_or));
  return not_found_or.value();
}

// Free function with move semantics to get the "found" content of a NotFoundOr object.
template <typename T>
[[nodiscard]] T&& GetFound(NotFoundOr<T>&& not_found_or) {
  ORBIT_CHECK(!IsNotFound(not_found_or));
  return std::move(not_found_or).value();
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_NOT_FOUND_OR_H_