// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_TYPEDEF_H_
#define ORBIT_BASE_TYPEDEF_H_

#include <absl/hash/hash.h>

#include <functional>
#include <type_traits>
#include <utility>

#include "OrbitBase/TypedefUtils.h"

namespace orbit_base {

// Strong typedef.
// It is parameterized by two types. `T`, which represents the type of the stored value, and `Tag_`,
// which allows to distinguish between different Typedefs. First, the user is expected to define a
// tag. Like `struct MyTypeTag {};`
// Also, a templated alias can come in handy
// ```
// template <typename T>
// using MyType = Typedef<MyTypeTag, T>;
// MyType<int> wrapped(1);
// ```
// One can access the underlying value with `*` and `->` operators. If `T` is non-`const`, the
// `const` and non-`const` overloads are provided. If `T` is const, only `const` overloads are
// provided.
//
// Assignment operators are provided independently of whether `T` is `const` or not.
//
// If `AbslHashValue` is implemented for `T`, it's also implemented for `Typedef<Tag, T>`.
//
// If `==` is implemented for `T` and `U`, operators `==` and `!=` are also implemented for
// `Typedef<Tag, T>` and `Typedef<Tag, U>`.
//
// If `<` is implemented for `T` and 'U', operators '<', '>', '<=', '>=' are also
// implemented for `Typedef<Tag, T>` and `Typedef<Tag, U>`. See TypedefTest.cpp for examples.
// See TypedefTest.cpp for examples.
template <typename Tag_, typename T>
class Typedef {
 public:
  using Tag = Tag_;
  using Value = std::remove_const_t<T>;

  [[nodiscard]] constexpr const T* operator->() const { return &value_; }

  template <typename = orbit_base_internal::EnableIfNonConst<T>>
  [[nodiscard]] constexpr T* operator->() {
    return &value_;
  }

  [[nodiscard]] constexpr const T& operator*() const& { return value_; }
  [[nodiscard]] constexpr T&& operator*() && { return std::move(value_); }
  [[nodiscard]] constexpr const T&& operator*() const&& { return std::move(value_); }

  template <typename = orbit_base_internal::EnableIfNonConst<T>>
  [[nodiscard]] constexpr T& operator*() & {
    return value_;
  }

  template <typename U, typename = orbit_base_internal::EnableIfUConvertibleToT<T, U>>
  constexpr explicit Typedef(U&& value) : value_(std::forward<U>(value)) {}

  template <typename... Args>
  constexpr explicit Typedef(std::in_place_t, Args&&... args)
      : value_(T(std::forward<T>(args)...)) {}

  constexpr Typedef() = default;

  constexpr Typedef(const Typedef& other) = default;
  constexpr Typedef(Typedef&& other) = default;

  template <typename U, typename = orbit_base_internal::EnableIfUConvertibleToT<T, U>>
  constexpr Typedef(const Typedef<Tag_, U>& other) : value_(*other) {}

  template <typename U, typename = orbit_base_internal::EnableIfUConvertibleToT<T, U>>
  constexpr Typedef(Typedef<Tag_, U>&& other) : value_(std::move(*other)) {}

  template <typename U, typename = orbit_base_internal::EnableIfUNotConvertibleToT<T, U>,
            typename = void>
  constexpr explicit Typedef(const Typedef<Tag_, U>& other) : value_(*other) {}

  template <typename U, typename = orbit_base_internal::EnableIfUNotConvertibleToT<T, U>,
            typename = void>
  constexpr explicit Typedef(Typedef<Tag_, U>&& other) : value_(std::move(*other)) {}

  constexpr Typedef& operator=(Typedef const&) = default;
  constexpr Typedef& operator=(Typedef&&) = default;

  template <typename U, typename = orbit_base_internal::EnableIfUConvertibleToT<T, U>>
  constexpr Typedef& operator=(const Typedef<Tag_, U>& other) {
    value_ = *other;
    return *this;
  }

  template <typename U, typename = orbit_base_internal::EnableIfUConvertibleToT<T, U>>
  constexpr Typedef& operator=(Typedef<Tag_, U>&& other) {
    value_ = std::move(*other);
    return *this;
  }

  template <typename H>
  friend H AbslHashValue(H h, const Typedef& wrapped) {
    return H::combine(std::move(h), *wrapped);
  }

  template <typename U>
  [[nodiscard]] friend bool operator==(const Typedef& lhs, const Typedef<Tag, U>& rhs) {
    return *lhs == *rhs;
  }
  template <typename U>
  [[nodiscard]] friend bool operator!=(const Typedef& lhs, const Typedef<Tag, U>& rhs) {
    return !(lhs == rhs);
  }
  template <typename U>
  [[nodiscard]] friend bool operator<(const Typedef& lhs, const Typedef<Tag, U>& rhs) {
    return *lhs < *rhs;
  }
  template <typename U>
  [[nodiscard]] friend bool operator>=(const Typedef& lhs, const Typedef<Tag, U>& rhs) {
    return !(lhs < rhs);
  }
  template <typename U>
  [[nodiscard]] friend bool operator>(const Typedef& lhs, const Typedef<Tag, U>& rhs) {
    return (lhs >= rhs) && (lhs != rhs);
  }
  template <typename U>
  [[nodiscard]] friend bool operator<=(const Typedef& lhs, const Typedef<Tag, U>& rhs) {
    return !(lhs > rhs);
  }

 private:
  Value value_;
};

template <typename Tag>
class Typedef<Tag, void> {};

// Say, we have a pair of typedefs.
// ```
// const MyType<int> kFirstWrapped(1);
// const MyType<int> kSecondWrapped(2);
// ```
// and also a callable (not necessarily a lambda)
// `auto add = [](int i, int j) { return i + j; };`
// Finally, we can use `Apply` function to apply `add` to the values stored in `kFirstWrapped` and
// `kSecondWrapped`. The returned value of the add will be wrapped in a typedef with the same
// `Tag_`.
// `const MyType<int> sum_wrapped = Apply(add, kFirstWrapped, kSecondWrapped);`
// Naturally, had we supplied the arguments wrapped in Typedefs of different `Tag_`, the code
// wouldn't compile.
// ```
// struct OtherTag {};
// using MyOtherType = Typedef<OtherTag, T>;
// const MyOtherType<int> kWrong(2);
// Apply(add, kFirstWrapped, kWrong); // ERROR!!!
// ```
// See TypedefTest.cpp for more examples.
template <typename Action, typename Arg, typename... Args>
auto LiftAndApply(Action&& action, Arg&& arg, Args&&... args) {
  using Tag = typename std::decay_t<Arg>::Tag;
  static_assert((std::is_same_v<Tag, typename std::decay_t<Args>::Tag> && ...),
                "Typedef tags don't match.");
  using R = decltype(std::invoke(std::forward<Action>(action), *std::forward<Arg>(arg),
                                 (*std::forward<Args>(args))...));
  if constexpr (!std::is_same_v<void, R>) {
    return Typedef<Tag, R>(std::invoke(std::forward<Action>(action), *std::forward<Arg>(arg),
                                       (*std::forward<Args>(args))...));
  } else {
    std::invoke(std::forward<Action>(action), *std::forward<Arg>(arg),
                (*std::forward<Args>(args))...);
    return Typedef<Tag, void>();
  }
}

template <typename TypedefType>
struct HasZeroMemoryOverhead
    : std::integral_constant<bool, sizeof(TypedefType) == sizeof(typename TypedefType::Value) &&
                                       std::alignment_of_v<TypedefType> ==
                                           std::alignment_of_v<typename TypedefType::Value>> {};

template <typename TypedefType>
inline constexpr bool kHasZeroMemoryOverheadV = HasZeroMemoryOverhead<TypedefType>::value;

}  // namespace orbit_base

#endif  // ORBIT_BASE_TYPEDEF_H_
