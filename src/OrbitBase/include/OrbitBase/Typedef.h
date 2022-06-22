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

// Strong typedef
template <typename Tag, typename T>
class Typedef {
 public:
  template <typename OtherTag, typename U>
  friend class Typedef;

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
  constexpr Typedef(const Typedef<Tag, U>& other) : value_(other.value_) {}

  template <typename U, typename = EnableIfUConvertibleToT<T, U>>
  constexpr Typedef(Typedef<Tag, U>&& other) : value_(std::move(other.value_)) {}

  template <typename U, typename = EnableIfUIsNotConvertibleToT<T, U>, typename = void>
  explicit Typedef(const Typedef<Tag, U>& other) : value_(other.value_) {}

  template <typename U, typename = EnableIfUIsNotConvertibleToT<T, U>, typename = void>
  explicit Typedef(Typedef<Tag, U>&& other) : value_(std::move(other.value_)) {}

  Typedef& operator=(Typedef const&) = default;
  Typedef& operator=(Typedef&&) = default;

  template <typename U, typename = EnableIfUConvertibleToT<T, U>>
  Typedef& operator=(const Typedef<Tag, U>& other) {
    value_ = other.value_;
    return *this;
  }

  template <typename U, typename = EnableIfUConvertibleToT<T, U>>
  Typedef& operator=(Typedef<Tag, U>&& other) {
    value_ = std::move(other.value_);
    return *this;
  }

 private:
  T value_;
};

template <typename Tag>
class Typedef<Tag, void> {};

template <typename>
struct TypedefTag {};

template <typename Tag, typename T>
struct TypedefTag<Typedef<Tag, T>> {
  using tag = Tag;
};

template <typename T>
using TypedefTagT = typename TypedefTag<T>::tag;

template <typename T, typename...>
struct AreSame : std::true_type {};

template <typename T, typename U, typename... TT>
struct AreSame<T, U, TT...>
    : std::integral_constant<bool, std::is_same<T, U>{} && AreSame<T, TT...>{}> {};

template <typename Head, typename... Tail>
struct First {
  using type = Head;
};

// `action` is a callable. `args` are its args wrapped in `Typedef`s  with the same `Tag`. The args
// are unwrapped, `action` is invoked on them. The returned value is wrapped in a `Typedef` with the
// same `Tag` and returned.
template <typename Action, typename... Args,
          typename Tag = typename First<TypedefTagT<std::decay_t<Args>>...>::type>
auto Apply(Action&& action, Args&&... args) {
  static_assert(AreSame<TypedefTagT<std::decay_t<Args>>...>{}, "Typedef tags don't match.");

  using R = decltype(std::invoke(std::forward<Action>(action), (*std::forward<Args>(args))...));
  if constexpr (!std::is_same_v<void, R>) {
    return Typedef<Tag, R>(
        std::invoke(std::forward<Action>(action), (*std::forward<Args>(args))...));
  } else {
    std::invoke(std::forward<Action>(action), (*std::forward<Args>(args))...);
    return Typedef<Tag, void>();
  }
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_TYPEDEF_H_
