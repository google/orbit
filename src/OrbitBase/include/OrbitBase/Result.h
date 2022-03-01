// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_RESULT_H_
#define ORBIT_BASE_RESULT_H_

#include <string>
#include <type_traits>

// Qt's MOC has some problems parsing outcome.hpp,
// but that's not a big deal as we can just exclude
// it from parsing without any side-effects.
#ifndef Q_MOC_RUN

#include <outcome.hpp>  // IWYU pragma: export

// Outcome has a versioned namespace scheme to allow
// mixing different versions of Outcome in the same
// project without having ABI problems.
//
// That's why we have to use a namespace alias.
//
// We used to have that alias definition in the conan
// package, but since we have been transitioning to the
// official conan package of Outcome, the namespace alias
// needs to move somewhere.
namespace outcome = OUTCOME_V2_NAMESPACE;

#endif  // Q_MOC_RUN

class [[nodiscard]] ErrorMessage final {
 public:
  ErrorMessage() = default;
  explicit ErrorMessage(std::string message) : message_(std::move(message)) {}

  // This implicit conversion constructor helps migrating code from outcome::result<T> to
  // ErrorMessageOr<T> because it allows OUTCOME_TRY to call functions that return outcome::result
  // in functions that return ErrorMessageOr. NOLINTNEXTLINE
  /* explicit(false) */ ErrorMessage(std::error_code ec) : message_(ec.message()) {}

  [[nodiscard]] const std::string& message() const { return message_; }

  [[nodiscard]] friend bool operator==(const ErrorMessage& lhs, const ErrorMessage& rhs) {
    return lhs.message_ == rhs.message_;
  }

  [[nodiscard]] friend bool operator!=(const ErrorMessage& lhs, const ErrorMessage& rhs) {
    return !(lhs == rhs);
  }

 private:
  std::string message_;
};

template <typename T, typename E>
using Result = outcome::result<T, E, outcome::policy::terminate>;

template <typename T>
class [[nodiscard]] ErrorMessageOr : public Result<T, ErrorMessage> {
 public:
  using Result<T, ErrorMessage>::Result;

  operator bool() = delete;
  operator bool() const = delete;
};

template <typename T>
struct IsErrorMessageOr : std::false_type {};

template <typename T>
struct IsErrorMessageOr<ErrorMessageOr<T>> : std::true_type {};

#endif  // ORBIT_BASE_RESULT_H_
