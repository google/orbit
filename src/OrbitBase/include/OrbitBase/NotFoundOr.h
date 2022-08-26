// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_NOT_FOUND_OR_H_
#define ORBIT_BASE_NOT_FOUND_OR_H_

#include <string>
#include <variant>

#include "OrbitBase/Logging.h"
#include "OrbitBase/VoidToMonostate.h"

namespace orbit_base {

// NotFound type that carries a message
struct NotFound {
  explicit NotFound(std::string message) : message(std::move(message)) {}
  std::string message;
};

// NotFoundOr can be used as the return type of a search operation, where the search can be
// unsuccessful and returns a message. Based on std::variant. Check whether a NotFoundOr object is
// a "not found" result with orbit_base::IsNotFound. Get the message with GetNotFoundMessage.
template <typename T>
using NotFoundOr = std::variant<VoidToMonostate_t<T>, NotFound>;

// Free function to check whether a NotFoundOr type is "not found". Abstracts
// std::holds_alternative
template <typename T>
[[nodiscard]] bool IsNotFound(const std::variant<T, NotFound>& not_found_or) {
  return std::holds_alternative<NotFound>(not_found_or);
}

// Free function to get the not found message of a NotFoundOr object.
template <typename T>
[[nodiscard]] const std::string& GetNotFoundMessage(const std::variant<T, NotFound>& not_found_or) {
  ORBIT_CHECK(IsNotFound(not_found_or));
  return std::get<NotFound>(not_found_or).message;
}

// Free function with move semantics to get the not found message of a NotFoundOr object.
template <typename T>
[[nodiscard]] std::string&& GetNotFoundMessage(std::variant<T, NotFound>&& not_found_or) {
  ORBIT_CHECK(IsNotFound(not_found_or));
  return std::move(std::get<NotFound>(std::move(not_found_or)).message);
}

// Free function to get the "found" content of a NotFoundOr object.
template <typename T>
[[nodiscard]] const T& GetFound(const std::variant<T, NotFound>& not_found_or) {
  ORBIT_CHECK(!IsNotFound(not_found_or));
  return std::get<T>(not_found_or);
}

// Free function with move semantics to get the "found" content of a NotFoundOr object.
template <typename T>
[[nodiscard]] T&& GetFound(std::variant<T, NotFound>&& not_found_or) {
  ORBIT_CHECK(!IsNotFound(not_found_or));
  return std::get<T>(std::move(not_found_or));
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_NOT_FOUND_OR_H_