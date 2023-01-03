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
// unsuccessful and returns a message. Check whether a NotFoundOr object is
// a "not found" result with orbit_base::IsNotFound. Get the message with GetNotFoundMessage.
//
// `NotFoundOr<T>` is implemented by privately deriving from `std::variant`. This allows us to take
// advantage of std::variant's carefully crafted constructors while still keeping std::variant an
// implementation detail. Implementing those constructors by hand would add a lot of complexity.
template <typename T>
class NotFoundOr : private std::variant<VoidToMonostate_t<T>, NotFound> {
 public:
  using Value = VoidToMonostate_t<T>;

  [[nodiscard]] bool HasValue() const { return this->index() == 0; }
  [[nodiscard]] bool IsNotFound() const { return !HasValue(); }

  [[nodiscard]] const Value& GetValue() const& { return std::get<0>(*this); }
  [[nodiscard]] const Value& GetValue() const&& { return std::get<0>(*this); }
  [[nodiscard]] Value GetValue() && { return std::get<0>(std::move(*this)); }

  [[nodiscard]] const NotFound& GetNotFound() const& { return std::get<1>(*this); }
  [[nodiscard]] const NotFound& GetNotFound() const&& { return std::get<1>(*this); }
  [[nodiscard]] NotFound GetNotFound() && { return std::get<1>(std::move(*this)); }

  using std::variant<Value, NotFound>::variant;
  using std::variant<Value, NotFound>::operator=;
};

// Free function to check whether a NotFoundOr type is "not found".
template <typename T>
[[nodiscard]] bool IsNotFound(const NotFoundOr<T>& not_found_or) {
  return not_found_or.IsNotFound();
}

// Free function to get the not found message of a NotFoundOr object.
template <typename T>
[[nodiscard]] const std::string& GetNotFoundMessage(const NotFoundOr<T>& not_found_or) {
  ORBIT_CHECK(IsNotFound(not_found_or));
  return not_found_or.GetNotFound().message;
}

// Free function with move semantics to get the not found message of a NotFoundOr object.
template <typename T>
[[nodiscard]] std::string GetNotFoundMessage(NotFoundOr<T>&& not_found_or) {
  ORBIT_CHECK(IsNotFound(not_found_or));
  return std::move(not_found_or).GetNotFound().message;
}

// Free function to get the "found" content of a NotFoundOr object.
template <typename T>
[[nodiscard]] const T& GetFound(const NotFoundOr<T>& not_found_or) {
  ORBIT_CHECK(!IsNotFound(not_found_or));
  return not_found_or.GetValue();
}

// Free function with move semantics to get the "found" content of a NotFoundOr object.
template <typename T>
[[nodiscard]] T GetFound(NotFoundOr<T>&& not_found_or) {
  ORBIT_CHECK(!IsNotFound(not_found_or));
  return std::move(not_found_or).GetValue();
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_NOT_FOUND_OR_H_