// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_ANY_ERROR_OF_H_
#define ORBIT_BASE_ANY_ERROR_OF_H_

#include <string>
#include <utility>
#include <variant>

#include "OrbitBase/ParameterPackTrait.h"

namespace orbit_base {

// A wrapper around `std::variant` holding one instance of multiple possible error types. It's
// mainly meant to be used with `Result` (`Result<T, AnyErrorOf<E1, E2>`) in cases where a function
// may return one of multiple possible error types. `AnyErrorOf<E1,E2>` has a `.message()` member
// function that returns the error message of the holding error by forwarding the call to the
// `.message()` function of the holding error.
//
// `AnyErrorOf` behaves like `std::variant` except for the following properties:
// 1. `ErrorTypes...` may not have duplicate types - all types must be unique.
// 2. An empty list of `ErrorTypes...` is not allowed.
// 3. All error types must have a `.message()` function that's either marked `const` or `static` and
//    that returns something that's convertible to `std::string`.
// 4. `AnyErrorOf<T1s...>` can be converted into `AnyErrorOf<T2s...>` if `T2s` contains at least all
//    the types present in `T1s`. The order of the types has no meaning.
// 5. `AnyErrorOf` can be directly compared (equality and inequality) to a value of one of its error
//    types if the given error type defines an equality comparison operator. The compared values are
//    considered equal if `AnyErrorOf` holds a value of the comparing error type and if the equality
//    comparison operator returns true.
template <typename... ErrorTypes>
class AnyErrorOf : public std::variant<ErrorTypes...> {
  using Base = std::variant<ErrorTypes...>;

 public:
  using Base::Base;
  using Base::operator=;

  static_assert(ParameterPackTrait<AnyErrorOf, ErrorTypes...>::kSize >= 1,
                "AnyError<> (AnyErrorOf with no error types) is not allowed.");

  static_assert(!ParameterPackTrait<AnyErrorOf, ErrorTypes...>::kHasDuplicates,
                "AnyError<ErrorTypes...> must not have duplicate error types.");

  template <typename... Types>
  constexpr static bool kCanBeConstructedFromTypesAndIsNotCopy = [] {
    constexpr ParameterPackTrait<AnyErrorOf, ErrorTypes...> kThisTrait{};
    constexpr ParameterPackTrait<AnyErrorOf, Types...> kOtherTrait{};

    // We return false if `AnyErrorOf<ErrorTypes...>` is the same type as `AnyErrorOf<Types...>`.
    // This avoids collisions with the copy and move constructors/assignment operators.
    if (kThisTrait == kOtherTrait) return false;

    // We only allow conversion from an instance of `AnyErrorOf<Types...>` with `Types` being a
    // subset of `ErrorTypes`.
    return kThisTrait.IsSubset(kOtherTrait);
  }();

  template <typename... Types>
  using EnableIfCanBeConstructedFromTypesAndIsNotCopy =
      std::enable_if_t<kCanBeConstructedFromTypesAndIsNotCopy<Types...>, int>;

  // Is true iff `Type` is in the `ErrorTypes...` parameter pack.
  template <typename Type>
  constexpr static bool kIsAnErrorType =
      ParameterPackTrait<AnyErrorOf, ErrorTypes...>::template kContains<Type>;

  template <typename Variant>
  auto ToBase(Variant&& other) {
    return std::visit(
        [](auto&& alternative) -> Base { return std::forward<decltype(alternative)>(alternative); },
        std::forward<Variant>(other));
  }

  // The following converting constructors/assignment operators allow conversion of any AnyErrorOf
  // type into a compatible AnyErrorOf type. An AnyErrorOf type is considered compatible
  // if its error type list is a super set of the other's error type list. The order of the types
  // doesn't matter though.
  //
  // Examples:
  //  - AnyErrorOf<E1> can be converted into AnyErrorOf<E1, E2> but not into AnyErrorOf<E2, E3>.
  //  - AnyErrorOf<E1, E2> can be converted into AnyErrorOf<E1, E3, E2>.
  //  - AnyErrorOf<E1, E2> can be converted into AnyErrorOf<E2, E1>.
  template <typename... Types, EnableIfCanBeConstructedFromTypesAndIsNotCopy<Types...> = 0>
  // NOLINTNEXTLINE(google-explicit-constructor)
  AnyErrorOf(const AnyErrorOf<Types...>& other) : Base{ToBase(other)} {}

  template <typename... Types, EnableIfCanBeConstructedFromTypesAndIsNotCopy<Types...> = 0>
  // NOLINTNEXTLINE(google-explicit-constructor)
  AnyErrorOf(AnyErrorOf<Types...>&& other) : Base{ToBase(std::move(other))} {}

  template <typename... Types, EnableIfCanBeConstructedFromTypesAndIsNotCopy<Types...> = 0>
  AnyErrorOf& operator=(const AnyErrorOf<Types...>& other) {
    *this = ToBase(other);
    return *this;
  }

  template <typename... Types, EnableIfCanBeConstructedFromTypesAndIsNotCopy<Types...> = 0>
  AnyErrorOf& operator=(AnyErrorOf<Types...>&& other) {
    *this = ToBase(std::move(other));
    return *this;
  }

  [[nodiscard]] std::string message() const {
    return std::visit([](const auto& error) { return std::string{error.message()}; }, *this);
  }

  // We allow transparent comparison with any of the error types.
  template <typename T, std::enable_if_t<kIsAnErrorType<T>, int> = 0>
  [[nodiscard]] friend bool operator==(const AnyErrorOf& lhs, const T& rhs) {
    return std::holds_alternative<T>(lhs) && std::get<T>(lhs) == rhs;
  }

  template <typename T, std::enable_if_t<kIsAnErrorType<T>, int> = 0>
  [[nodiscard]] friend bool operator==(const T& lhs, const AnyErrorOf& rhs) {
    return std::holds_alternative<T>(lhs) && std::get<T>(lhs) == rhs;
  }

  template <typename T, std::enable_if_t<kIsAnErrorType<T>, int> = 0>
  [[nodiscard]] friend bool operator!=(const AnyErrorOf& lhs, const T& rhs) {
    return !(lhs == rhs);
  }

  template <typename T, std::enable_if_t<kIsAnErrorType<T>, int> = 0>
  [[nodiscard]] friend bool operator!=(const T& lhs, const AnyErrorOf& rhs) {
    return !(lhs == rhs);
  }
};
}  // namespace orbit_base

#endif  // ORBIT_BASE_ANY_ERROR_OF_H_
