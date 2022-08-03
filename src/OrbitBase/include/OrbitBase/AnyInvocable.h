// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_ANY_INVOCABLE_H_
#define ORBIT_BASE_ANY_INVOCABLE_H_

#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>
#include <type_traits>

namespace orbit_base {

template <typename>
class AnyInvocable;

// AnyInvocable is a polymorphic function wrapper, just like std::function. But unlike std::function
// and as the name suggests, it can wrap everything that fulfills the Invocable concept. That
// means the wrapped function objects don't need to be copy-constructible. It's enough for them to
// be movable.
//
// Usage: Think std::function when using AnyInvocable.
// AnyInvocable<int(int)> invocable{[](int val) { return 42 * val; }};
// std::cout << invocable(2) << std::endl; // Outputs 48
template <typename R, typename... Args>
class AnyInvocable<R(Args...)> {
  struct Base {
    constexpr Base() = default;
    constexpr Base(Base&&) = default;
    constexpr Base& operator=(Base&&) = default;

    constexpr Base(const Base&) = delete;
    constexpr Base& operator=(const Base&) = delete;

    virtual R invoke(Args...) = 0;
    virtual ~Base() = default;
  };

  template <typename T>
  class Storage final : public Base {
    T value_;

   public:
    using Base::Base;

    template <typename... Urgs>
    constexpr explicit Storage(std::in_place_t, Urgs... args)
        : value_{std::forward<Urgs>(args)...} {}

    constexpr explicit Storage(const T& value) : value_(value) {}
    constexpr explicit Storage(T&& value) : value_(std::move(value)) {}

    constexpr R invoke(Args... args) override {
      return std::invoke(value_, std::forward<Args>(args)...);
    }
  };

  std::unique_ptr<Base> storage_;

 public:
  template <typename F, typename = std::enable_if_t<!std::is_same_v<std::decay_t<F>, AnyInvocable>>>
  constexpr explicit AnyInvocable(F&& func)
      : storage_{std::make_unique<Storage<std::decay_t<F>>>(std::forward<F>(func))} {}

  constexpr explicit operator bool() const { return storage_ != nullptr; }

  constexpr R operator()(Args... args) { return storage_->invoke(std::forward<Args>(args)...); }

  friend bool operator==(const AnyInvocable& lhs, std::nullptr_t) {
    return !static_cast<bool>(lhs);
  }
  friend bool operator==(std::nullptr_t, const AnyInvocable& rhs) {
    return !static_cast<bool>(rhs);
  }
  friend bool operator!=(const AnyInvocable& lhs, std::nullptr_t rhs) { return !(lhs == rhs); }
  friend bool operator!=(std::nullptr_t lhs, const AnyInvocable& rhs) { return !(lhs == rhs); }
};

}  // namespace orbit_base

#endif  // ORBIT_BASE_ANY_INVOCABLE_H_