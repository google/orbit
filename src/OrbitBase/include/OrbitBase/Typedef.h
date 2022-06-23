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

template <typename T, typename U>
using EnableIfUIsNotConvertibleToT = std::enable_if_t<!std::is_convertible_v<U, T>>;

// Strong typedef.
// It is parameterized by two types. `T`, which represents the type of the stored value and `Tag_`,
// which allows to distinguish between different Typedefs. First, the user is expected to define a
// tag. Like `struct TestTag {};`
// Also, a templated alias can come in handy
// ```
// template <typename T>
// using Wrapper = Typedef<TestTag, T>;
// ```
// Now we can instantiate the typedefs
// ```
// const Wrapper<int> kFirstWrapped(1);
// const Wrapper<int> kSecondWrapped(2);
// ```
// and define a callable (not necessarily a lambda)
// `auto add = [](int i, int j) { return i + j; };`
// Finally, we can use `Apply` function to apply `add` to the values stored in `kFirstWrapped` and
// `kSecondWrapped`. The returned value of the add will be wrapped in a typedef with the same
// `Tag_`.
// `const Wrapper<int> sum_wrapped = Apply(add, kFirstWrapped, kSecondWrapped);`
// Naturally, had we supplied the arguments wrapped in Typedefs of different `Tag_`, the code
// wouldn't compile.
// ```
// struct OtherTag {};
// using OtherWrapper = Typedef<OtherTag, T>;
// const OtherWrapper<int> kWrong(2);
// Apply(add, kFirstWrapped, kWrong); // ERROR!!!
// ```
// See TypedefTest.cpp for more examples.
template <typename Tag_, typename T>
class Typedef {
 public:
  using Tag = Tag_;

  [[nodiscard]] constexpr const T* operator->() const { return &value_; }
  [[nodiscard]] constexpr T* operator->() { return &value_; }

  [[nodiscard]] constexpr const T& operator*() const& { return value_; }
  [[nodiscard]] constexpr T&& operator*() && { return std::move(value_); }
  [[nodiscard]] constexpr const T&& operator*() const&& { return std::move(value_); }
  [[nodiscard]] constexpr T& operator*() & { return value_; }

  template <typename U, typename = EnableIfUConvertibleToT<T, U>>
  constexpr explicit Typedef(U&& value) : value_(std::forward<U>(value)) {}

  template <typename... Args>
  constexpr explicit Typedef(std::in_place_t, Args&&... args)
      : value_(T(std::forward<T>(args)...)) {}

  constexpr Typedef(Typedef&& other) = default;

  template <typename U, typename = EnableIfUConvertibleToT<T, U>>
  constexpr Typedef(const Typedef<Tag_, U>& other) : value_(*other) {}

  template <typename U, typename = EnableIfUConvertibleToT<T, U>>
  constexpr Typedef(Typedef<Tag_, U>&& other) : value_(std::move(*other)) {}

  template <typename U, typename = EnableIfUIsNotConvertibleToT<T, U>, typename = void>
  constexpr explicit Typedef(const Typedef<Tag_, U>& other) : value_(*other) {}

  template <typename U, typename = EnableIfUIsNotConvertibleToT<T, U>, typename = void>
  constexpr explicit Typedef(Typedef<Tag_, U>&& other) : value_(std::move(*other)) {}

  constexpr Typedef& operator=(Typedef const&) = default;
  constexpr Typedef& operator=(Typedef&&) = default;

  template <typename U, typename = EnableIfUConvertibleToT<T, U>>
  constexpr Typedef& operator=(const Typedef<Tag_, U>& other) {
    value_ = *other;
    return *this;
  }

  template <typename U, typename = EnableIfUConvertibleToT<T, U>>
  constexpr Typedef& operator=(Typedef<Tag_, U>&& other) {
    value_ = std::move(*other);
    return *this;
  }

 private:
  T value_;
};

template <typename Tag>
class Typedef<Tag, void> {};

// `action` is a callable. `args` are its args wrapped in `Typedef`s  with the same `Tag`. The args
// are unwrapped, `action` is invoked on them. The returned value is wrapped in a `Typedef` with the
// same `Tag` and returned.
template <typename Action, typename Arg, typename... Args>
auto Apply(Action&& action, Arg&& arg, Args&&... args) {
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

}  // namespace orbit_base

#endif  // ORBIT_BASE_TYPEDEF_H_
