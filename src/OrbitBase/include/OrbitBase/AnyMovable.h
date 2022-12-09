// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_ANY_MOVABLE_H_
#define ORBIT_BASE_ANY_MOVABLE_H_

#include <memory>
#include <type_traits>

namespace orbit_base {

// AnyMovable is a type-safe container for values of any type that fulfills the Movable concept.
// It's very similar to std::any but does not require the type to be copy-constructible.
// Hence AnyMovable itself is a move-only type.
//
// Usage: The API is pretty similar to std::any. In most of the places the naming styles has been
// adjusted to Google style. Store a value by assignment or Emplace and use any_movable_cast to get
// it back out.
//
// AnyMovable m = std::make_unique<int>(42);
// auto other = std::move(m);
// std::cout << std::boolalpha << m.HasValue() << other.HasValue(); // Prints "falsetrue"
//
// int* int_ptr = any_movable_cast<int>(&other);
// std::cout << int_ptr; // Prints 0, since `other` does not hold an `int`.
//
// std::unique_ptr<int>* unique_int_ptr = any_movable_cast<std::unique_ptr<int>>(&other);
// ORBIT_CHECK(unique_int_ptr != nulllptr);
// std::cout << **unique_int_ptr; // Prints "42"
class AnyMovable {
  struct Base {
    constexpr Base() = default;
    constexpr Base(Base&&) = default;
    Base& operator=(Base&&) = default;

    constexpr Base(const Base&) = delete;
    constexpr Base& operator=(const Base&) = delete;

    virtual void* AccessValue() = 0;
    virtual ~Base() = default;
  };

  template <typename T>
  class Storage final : public Base {
    T value_;

   public:
    using Base::Base;

    template <typename... Args>
    constexpr explicit Storage(std::in_place_t, Args&&... args)
        : value_{std::forward<Args>(args)...} {}

    constexpr explicit Storage(const T& value) : value_(value) {}
    constexpr explicit Storage(T&& value) : value_(std::move(value)) {}

    [[nodiscard]] void* AccessValue() override { return static_cast<void*>(&value_); }
  };

  std::unique_ptr<Base> storage_;
  const std::type_info* type_info_ = nullptr;

 public:
  explicit AnyMovable() = default;

  template <typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, AnyMovable>>>
  explicit AnyMovable(T&& value)
      : storage_{std::make_unique<Storage<std::decay_t<T>>>(std::forward<T>(value))},
        type_info_{&typeid(value)} {}

  template <typename T, typename... Args>
  explicit AnyMovable(std::in_place_type_t<T>, Args&&... args)
      : storage_{std::make_unique<Storage<std::decay_t<T>>>(std::in_place,
                                                            std::forward<Args>(args)...)},
        type_info_{&typeid(std::decay_t<T>)} {}

  void Reset() { storage_.reset(); }

  [[nodiscard]] bool HasValue() const { return storage_ != nullptr; }

  template <typename ValueType, typename... Args>
  std::decay_t<ValueType>& Emplace(Args&&... args) {
    storage_ = std::make_unique<Storage<ValueType>>(std::in_place, std::forward<Args>(args)...);
    type_info_ = &typeid(ValueType);
    return *static_cast<ValueType*>(storage_->AccessValue());
  }

  [[nodiscard]] const std::type_info& type() const { return *type_info_; }

  template <typename T>
  friend T* any_movable_cast(AnyMovable*);

  template <typename T>
  friend const T* any_movable_cast(const AnyMovable*);
};

// This is deviating from Google's naming style to be in line with `static_cast` and others.
// Abseil is doing the same with `abseil::bit_cast`.
template <typename T>
[[nodiscard]] T* any_movable_cast(AnyMovable* movable) {
  if (!(movable->type() == typeid(T))) return nullptr;

  return static_cast<T*>(movable->storage_->AccessValue());
}

template <typename T>
[[nodiscard]] const T* any_movable_cast(const AnyMovable* movable) {
  if (!(movable->type() == typeid(T))) return nullptr;

  return static_cast<const T*>(movable->storage_->AccessValue());
}

template <typename T, typename... Args>
[[nodiscard]] AnyMovable MakeAnyMovable(Args&&... args) {
  return AnyMovable{std::in_place_type<T>, std::forward<Args>(args)...};
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_ANY_MOVABLE_H_