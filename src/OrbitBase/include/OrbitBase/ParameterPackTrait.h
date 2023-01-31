// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_PARAMETER_PACK_TRAIT_H_
#define ORBIT_BASE_PARAMETER_PACK_TRAIT_H_

#include <stddef.h>

#include <tuple>
#include <type_traits>

namespace orbit_base {
template <template <typename...> typename, typename...>
struct ParameterPackTrait;
}

namespace orbit_base_internal {

// This is a helper type that is needed to implement `ParameterPackTrait::RemoveDuplicateTypes()`.
// It is based on recursion - AFAIK there is currently no other way to implement this.
// The recursion depth is O(N), the number of type comparisons is O(N^2). This is far from great,
// but since our use cases are usually N \in {1, 2, 3}, it's also not that concerning.
template <typename Output, typename Input>
struct DeduplicatedTuple;

template <typename... OutputTypes, typename FirstInput, typename... OtherInputTypes>
struct DeduplicatedTuple<std::tuple<OutputTypes...>, std::tuple<FirstInput, OtherInputTypes...>> {
  using type = typename std::conditional_t<
      orbit_base::ParameterPackTrait<std::tuple, OutputTypes...>::template Contains<FirstInput>(),
      typename DeduplicatedTuple<std::tuple<OutputTypes...>, std::tuple<OtherInputTypes...>>::type,
      typename DeduplicatedTuple<std::tuple<OutputTypes..., FirstInput>,
                                 std::tuple<OtherInputTypes...>>::type>;
};

template <typename... OutputTypes>
struct DeduplicatedTuple<std::tuple<OutputTypes...>, std::tuple<>> {
  using type = std::tuple<OutputTypes...>;
};

template <typename Tuple>
using DeduplicatedTuple_t = typename DeduplicatedTuple<std::tuple<>, Tuple>::type;

template <typename Tuple>
struct TupleToParameterPackTraits;

template <typename... Types>
struct TupleToParameterPackTraits<std::tuple<Types...>> {
  using type = orbit_base::ParameterPackTrait<std::tuple, Types...>;
};

template <typename Tuple>
using TupleToParameterPackTraits_t = typename TupleToParameterPackTraits<Tuple>::type;

}  // namespace orbit_base_internal

namespace orbit_base {

// This is a trait that helps you deal with a parameter pack `Types...`. It also captures a variadic
// holder template type (any template that takes a parameter pack as its argument - like
// `std::variant` or `std::tuple`). For example `ParameterPackTrait<std::tuple, int, char>`
// describes the type `std::tuple<int, char>`.
//
// An instance of this trait can be constructed either through default construction or from an
// existing value using the `MakeParameterPackTrait` constexpr factory function below. Look there
// for more details.
//
// The trait provides a variety of helper functions to deal with parameter packs. They are
// documented below in the code.
template <template <typename...> typename VariadicHolder, typename... Types>
struct ParameterPackTrait {
  // Returns true iff the given type `T` is part of the parameter pack.
  template <typename T>
  [[nodiscard]] constexpr static bool Contains() {
    return (std::is_same_v<Types, T> || ...);
  }

  // Return true iff the types listed in `Others...` is a subset of the parameter pack.
  template <typename... Others>
  [[nodiscard]] constexpr static bool IsSubset() {
    return (Contains<Others>() && ...);
  }

  // Return true iff the types listed in `Others...` is a subset of the parameter pack.
  template <typename... Others>
  [[nodiscard]] constexpr static bool IsSubset(
      const ParameterPackTrait<VariadicHolder, Others...>& /* other*/) {
    return (Contains<Others>() && ...);
  }

  // Returns a `ParameterPackTrait` object where duplicate types in `Types` have been removed. The
  // order of types is not changed and it is guaranteed that the first appearance of a
  // distinct type in `Types` remains.
  // Note that this function is rather heavy on type instantiations and can slow down your build
  // when used with a large parameter pack.
  [[nodiscard]] constexpr static auto RemoveDuplicateTypes() {
    return orbit_base_internal::TupleToParameterPackTraits_t<
               orbit_base_internal::DeduplicatedTuple_t<std::tuple<Types...>>>{}
        .template ChangeVariadicHolder<VariadicHolder>();
  }

  // Constructs a value of type `VariadicHolder<Types...>` from the given arguments.
  template <typename... Args>
  [[nodiscard]] constexpr static VariadicHolder<Types...> ConstructValue(Args&&... args) {
    return {std::forward<Args>(args)...};
  }

  // With `decltype(trait.ToType())` you will get access to the type that this trait represents.
  // If your intention is to construct a new object, then `ConstructValue` from above might be
  // cleaner.
  [[nodiscard]] constexpr static VariadicHolder<Types...> ToType() {
    // Note that `std::declval` can only appear in an unevaluated context, so a call of `ToType`
    // must be wrapped in a `decltype(...)` expression.
    return std::declval<VariadicHolder<Types...>>();
  }

  // Constructs a new trait object that is a copy of the current object, but a list of types is
  // appended to the parameter pack.
  //
  // There is also an overload that accepts an instance of another trait object with the same //
  // variadic value type. The parameter pack of this other trait objects is appended to the
  // parameter pack of the current trait object.
  template <typename... AdditionalTypes>
  [[nodiscard]] constexpr static ParameterPackTrait<VariadicHolder, Types..., AdditionalTypes...>
  AppendTypes() {
    return {};
  }

  // Overload to the previous function. Look there for details.
  template <typename... AdditionalTypes>
  [[nodiscard]] constexpr static ParameterPackTrait<VariadicHolder, Types..., AdditionalTypes...>
  AppendTypes(
      const ParameterPackTrait<VariadicHolder, AdditionalTypes...>& /* unused_trait_object */) {
    return {};
  }

  // Constructs a new trait object with a different variadic value type.
  //
  // For example that allows you to go from a trait that describes `std::tuple<int, char>` to a
  // trait that describes `std::variant<int, char>`.
  template <template <typename...> typename NewVariadicHolder>
  [[nodiscard]] constexpr static ParameterPackTrait<NewVariadicHolder, Types...>
  ChangeVariadicHolder() {
    return {};
  }

  // Returns the number of types in the parameter pack.
  [[nodiscard]] constexpr static size_t Size() { return sizeof...(Types); }

  // Returns true iff there are duplicate types in the parameter pack. Note that this function calls
  // `RemoveDuplicateTypes` in its implementation and the same warning about extended compile times
  // applies here as well.
  [[nodiscard]] constexpr static bool HasDuplicates() {
    return RemoveDuplicateTypes().Size() < Size();
  }

  // These constexpr comparison operators allow to compare constexpr values instead of types which
  // leads to more idiomatic unit testing code.
  template <template <typename...> typename Other, typename... OtherTypes>
  [[nodiscard]] friend constexpr bool operator==(
      const ParameterPackTrait& /* lhs */,
      const ParameterPackTrait<Other, OtherTypes...> /* rhs */) {
    // Two ParameterPackTraits are considered equal if their types match.
    return std::is_same_v<ParameterPackTrait, ParameterPackTrait<Other, OtherTypes...>>;
  }

  template <template <typename...> typename Other, typename... OtherTypes>
  [[nodiscard]] friend constexpr bool operator!=(
      const ParameterPackTrait& lhs, const ParameterPackTrait<Other, OtherTypes...> rhs) {
    return !(lhs == rhs);
  }
};

// MakeParameterPackTrait creates a trait object - deduced from a value.
//
// - We support full deduction. In that case the value's type must be an instantiation of a variadic
//   template, in other words an instantiation of `Holder<Types...>`.
// - We also support partial deduction. In that case the Holder template must be provided explicitly
//   and the `Types` pack will be deduced from the value. If the value's type is an instantiation of
//   `Holder<Others...>` then `Others...` will become the pack. If the value's type is NOT an
//   instantiation of `Holder`, then the pack will consist of a single type `decltype(value)`.
//   Consider the following examples:
//   `MakePa...(variant<int, char>{})` returns `ParameterPackTrait<variant, int, char>{}`.
//   `MakePa...<variant>(variant<int, char>{})` returns `ParameterPackTrait<variant, int, char>{}`.
//   `MakePa...<variant>(0.0f)` returns `ParameterPackTrait<variant, float>{}`.
//
//   This is useful in particular for generic code. It allows to construct a type that's always an
//   instance of `Holder` while avoiding double wrapping (`Holder<Holder<..>>`).
template <template <typename...> typename VariadicHolder, typename... Types>
constexpr static ParameterPackTrait<VariadicHolder, Types...> MakeParameterPackTrait(
    const VariadicHolder<Types...>& /* unused_value */) {
  return {};
}

template <template <typename...> typename VariadicHolder, typename Type>
constexpr static ParameterPackTrait<VariadicHolder, Type> MakeParameterPackTrait(
    const Type& /* unused_value */) {
  return {};
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_PARAMETER_PACK_TRAIT_H_
