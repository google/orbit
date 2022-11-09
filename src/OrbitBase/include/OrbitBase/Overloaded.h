// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_OVERLOADED_H_
#define ORBIT_BASE_OVERLOADED_H_

#include <functional>
#include <type_traits>
#include <utility>

namespace orbit_base_internal {

// The class wraps a function pointer, providing `operator()`
template <typename FunctionPtr>
class FunctionPtrWithParen {
 public:
  FunctionPtrWithParen(FunctionPtr t) : function_ptr_(t) {}  // NOLINT(google-explicit-constructor)

  template <typename... In>
  auto operator()(In&&... in) const
      -> decltype(std::invoke(std::declval<FunctionPtr>(), std::forward<In>(in)...)) {
    return std::invoke(function_ptr_, std::forward<In>(in)...);
  }

 private:
  FunctionPtr function_ptr_;
};

// Compile-time check if `operator()` is defined for `T`
template <typename T, typename = std::void_t<>>
struct HasOperatorParen : std::false_type {};

template <typename T>
struct HasOperatorParen<T, std::void_t<decltype(&T::operator())>> : std::true_type {};

template <typename T>
inline constexpr bool kHasOperatorParenV = HasOperatorParen<T>::value;

// `T` if `operator()` is defined, otherwise `FunctionPtrWrapper<T>`
template <typename T>
using WithParen = std::conditional_t<kHasOperatorParenV<T>, T, FunctionPtrWithParen<T>>;

}  // namespace orbit_base_internal

namespace orbit_base {

// Extended `overloaded` trick from https://en.cppreference.com/w/cpp/utility/variant/visit
// Also handles function pointers.
template <typename... Ts>
struct overloaded : orbit_base_internal::WithParen<Ts>... {
  using orbit_base_internal::WithParen<Ts>::operator()...;
};

template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

}  // namespace orbit_base

#endif  // ORBIT_BASE_OVERLOADED_H_