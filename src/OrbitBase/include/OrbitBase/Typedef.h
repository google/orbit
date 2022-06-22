// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_TYPEDEF_H_
#define ORBIT_BASE_TYPEDEF_H_

#include <functional>
#include <type_traits>
#include <utility>

namespace orbit_base {

template <typename T, typename U>
using EnableIfUConvertibleToT = std::enable_if_t<std::is_convertible_v<U, T>>;

// Strong typedef
template <typename Tag, typename T>
class Typedef {
 public:
  [[nodiscard]] constexpr const T* operator->() const { return &value_; }
  [[nodiscard]] constexpr T* operator->() { return &value_; }

  [[nodiscard]] constexpr const T& operator*() const& { return value_; }
  [[nodiscard]] constexpr T&& operator*() && { return std::move(value_); }
  [[nodiscard]] constexpr const T&& operator*() const&& { return std::move(value_); }
  [[nodiscard]] constexpr T& operator*() & { return value_; }

  template <typename U, typename = EnableIfUConvertibleToT<T, U>>
  constexpr explicit Typedef(U&& value) : value_(std::forward<U>(value)) {}

  template <typename... Args>
  constexpr explicit Typedef(std::in_place_t, Args&&... args) : value_(T(std::forward<T>(args)...)) {}

  constexpr Typedef(Typedef&& other) = default;

 private:
  T value_;
};

template <typename Tag>
class Typedef<Tag, void> {};

// `action` is a callable that takes const-refs or values. `args` are the args wrapped in `Typedef`s
// with the same `Tag`. The args are unwrapped, `action` is invoked on them. The returned value is
// wrapped in a `Typedef` with the same `Tag` and returned.
template <typename Tag, typename Action, typename... Args,
          typename Return = std::invoke_result_t<Action, Args...>,
          typename = std::enable_if_t<!std::is_void_v<Return>>>
[[nodiscard]] Typedef<Tag, Return> Apply(Action&& action, const Typedef<Tag, Args>&... args) {
  return Typedef<Tag, Return>(std::invoke(std::forward<Action>(action), (*args)...));
}

// Overload for void functions.
template <typename Tag, typename Action, typename... Args,
          typename Return = std::invoke_result_t<Action, Args...>,
          typename = std::enable_if_t<std::is_void_v<Return>>>
Typedef<Tag, void> Apply(Action&& action, const Typedef<Tag, Args>&... args) {
  std::invoke(std::forward<Action>(action), (*args)...);
  return Typedef<Tag, void>();
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_TYPEDEF_H_
