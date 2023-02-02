// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>
#include <tuple>
#include <type_traits>
#include <variant>

#include "OrbitBase/ParameterPackTrait.h"

namespace orbit_base {

// All these tests are compile-time tests. Technically they don't need to be put in gtest TEST()
// directives. But it gives them some structure.

TEST(ParameterPackTrait, Size) {
  // Empty packs are supported
  static_assert(ParameterPackTrait<std::variant>::kSize == 0);

  static_assert(ParameterPackTrait<std::variant, int>::kSize == 1);
  static_assert(ParameterPackTrait<std::variant, int, float>::kSize == 2);

  // Duplicate types count as distinct pack elements - it's not a set
  static_assert(ParameterPackTrait<std::variant, int, int, int>::kSize == 3);
}

TEST(ParameterPackTrait, Contains) {
  static_assert(ParameterPackTrait<std::variant, int, float, double>::kContains<int>);
  static_assert(!ParameterPackTrait<std::variant, int, float, double>::kContains<char>);

  // Contains returns true even if the type is more than once in the pack
  static_assert(ParameterPackTrait<std::variant, int, float, double, int>::kContains<int>);
}

TEST(ParameterPackTrait, IsSubset) {
  // An empty pack is always a subset of any other pack, including an empty pack
  static_assert(ParameterPackTrait<std::variant, int, float, double>::IsSubset<>());
  static_assert(ParameterPackTrait<std::variant>::IsSubset<>());

  // Works for single elements
  static_assert(ParameterPackTrait<std::variant, int, float, double>::IsSubset<int>());
  static_assert(ParameterPackTrait<std::variant, int, float, double>::IsSubset(
      ParameterPackTrait<std::variant, int>{}));

  // Works for packs with unique elements (sets)
  static_assert(ParameterPackTrait<std::variant, int, float, double>::IsSubset<int, double>());
  static_assert(ParameterPackTrait<std::variant, int, float, double>::IsSubset(
      ParameterPackTrait<std::variant, int, double>{}));

  // Also returns true when sets are equal
  static_assert(
      ParameterPackTrait<std::variant, int, float, double>::IsSubset<float, int, double>());

  // Returns false if single element doesn't match
  static_assert(!ParameterPackTrait<std::variant, int, float, double>::IsSubset<char>());

  // Returns false if any single element doesn't match
  static_assert(!ParameterPackTrait<std::variant, int, float, double>::IsSubset<int, char>());

  // Returns false independent of the position of the non-matching element in the pack
  static_assert(
      !ParameterPackTrait<std::variant, int, float, double>::IsSubset<int, char, double>());
  static_assert(
      !ParameterPackTrait<std::variant, int, float, double>::IsSubset<char, float, int, double>());

  // The packs are treated as sets - so duplicate elements are not considered. Hence `<int, int>` is
  // a subset of `<int>`.
  static_assert(ParameterPackTrait<std::variant, int>::IsSubset<int, int>());
}

TEST(ParameterPackTrait, ToType) {
  static_assert(std::is_same_v<ParameterPackTrait<std::tuple, int, float, double>::Type,
                               std::tuple<int, float, double>>);
  static_assert(std::is_same_v<ParameterPackTrait<std::variant, int, float, double>::Type,
                               std::variant<int, float, double>>);
}

TEST(ParameterPackTrait, RemoveDuplicateTypes) {
  static_assert(ParameterPackTrait<std::variant, int, int>::DuplicateTypesRemoved{} ==
                ParameterPackTrait<std::variant, int>{});
  static_assert(
      ParameterPackTrait<std::variant, int, float, int, double, int>::DuplicateTypesRemoved{} ==
      ParameterPackTrait<std::variant, int, float, double>{});
  static_assert(
      ParameterPackTrait<std::variant, int, float, int, double, int>::DuplicateTypesRemoved{} !=
      ParameterPackTrait<std::variant, float, int, double>{});
}

TEST(ParameterPackTrait, HasDuplicates) {
  // No duplicates in empty pack
  static_assert(!ParameterPackTrait<std::variant>::kHasDuplicates);

  // No duplicates in single element pack
  static_assert(!ParameterPackTrait<std::variant, int>::kHasDuplicates);

  // Duplicates in pack with 2 identical types
  static_assert(ParameterPackTrait<std::variant, int, int>::kHasDuplicates);

  // No duplicates in pack with 2 distinct types
  static_assert(!ParameterPackTrait<std::variant, int, char>::kHasDuplicates);

  // Order doesn't matter - Duplicates still found
  static_assert(ParameterPackTrait<std::variant, int, char, int>::kHasDuplicates);
}

TEST(ParameterPackTrait, MakeParameterPackTrait) {
  // MakeParameterPackTrait deduces the variadic value template and the pack automatically when a
  // value that matches the type construct `Holder<Pack...>` is passed in.
  static_assert(MakeParameterPackTrait(std::variant<int, float, double>{}) ==
                ParameterPackTrait<std::variant, int, float, double>{});

  // Template parameters can also be partially provided, while a value is passed in. If this value
  // matches the given variadic template the pack is deduced as shown before.
  static_assert(MakeParameterPackTrait<std::variant>(std::variant<int, float, double>{}) ==
                ParameterPackTrait<std::variant, int, float, double>{});

  // If the provided value does not match the given variadic template, the value is considered the
  // only type in the pack. This is useful for generic code.
  static_assert(MakeParameterPackTrait<std::variant>(std::tuple<int, float, double>{}) ==
                ParameterPackTrait<std::variant, std::tuple<int, float, double>>{});
  static_assert(MakeParameterPackTrait<std::variant>(int{}) ==
                ParameterPackTrait<std::variant, int>{});
}

TEST(ParameterPackTrait, AppendTypes) {
  // variant<int, double, float> + variant<int, char> -> variant<int, double, float, int, char>

  // NOLINTNEXTLINE(readability-static-accessed-through-instance)
  static_assert(ParameterPackTrait<std::variant, int, double, float>{}.AppendTypes<int, char>() ==
                ParameterPackTrait<std::variant, int, double, float, int, char>{});

  // NOLINTNEXTLINE(readability-static-accessed-through-instance)
  static_assert(ParameterPackTrait<std::variant, int, double, float>{}.AppendTypes(
                    ParameterPackTrait<std::variant, int, char>{}) ==
                ParameterPackTrait<std::variant, int, double, float, int, char>{});

  // variant<int, int, double, float> + variant<int, char, int> -> variant<int, int, double, float,
  // int, char, int>

  static_assert(  // NOLINTNEXTLINE(readability-static-accessed-through-instance)
      ParameterPackTrait<std::variant, int, int, double, float>{}.AppendTypes<int, char, int>() ==
      ParameterPackTrait<std::variant, int, int, double, float, int, char, int>{});

  static_assert(  // NOLINTNEXTLINE(readability-static-accessed-through-instance)
      ParameterPackTrait<std::variant, int, int, double, float>{}.AppendTypes(
          ParameterPackTrait<std::variant, int, char, int>{}) ==
      ParameterPackTrait<std::variant, int, int, double, float, int, char, int>{});

  // variant<int, int, int> + variant<int, int, int> -> variant<int>

  // NOLINTNEXTLINE(readability-static-accessed-through-instance)
  static_assert(ParameterPackTrait<std::variant, int, int, int>{}.AppendTypes<int, int, int>() ==
                ParameterPackTrait<std::variant, int, int, int, int, int, int>{});

  // NOLINTNEXTLINE(readability-static-accessed-through-instance)
  static_assert(ParameterPackTrait<std::variant, int, int, int>{}.AppendTypes(
                    ParameterPackTrait<std::variant, int, int, int>{}) ==
                ParameterPackTrait<std::variant, int, int, int, int, int, int>{});
}

}  // namespace orbit_base
