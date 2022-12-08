// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <vector>

#include "OrbitBase/Sort.h"

namespace {
struct Struct {
  int value;
  int key;

  friend bool operator==(const Struct& a, const Struct& b) {
    return a.value == b.value && a.key == b.key;
  }
};

constexpr auto kProjection = [](const Struct& s) -> int { return s.value; };
}  // namespace

namespace orbit_base {

using testing::ElementsAreArray;

const std::vector<Struct> structs = {{5, 1}, {2, 2}, {3, 5}, {2, 3}, {2, 4},
                                     {1, 6}, {2, 7}, {2, 8}, {2, 9}};

template <typename MySort, typename StdSort>
static void ExpectSortIsCorrect(MySort my_sort, StdSort std_sort) {
  std::vector<Struct> structs_actual_sorted = structs;
  my_sort(structs_actual_sorted);

  std::vector<Struct> structs_expected_sorted = structs;
  std_sort(structs_expected_sorted);

  EXPECT_THAT(structs_actual_sorted, ElementsAreArray(structs_expected_sorted));
}

TEST(SortTest, SortIsCorrect) {
  ExpectSortIsCorrect(
      [](auto& structs) { orbit_base::sort(std::begin(structs), std::end(structs), kProjection); },
      [](auto& structs) {
        std::sort(std::begin(structs), std::end(structs),
                  [](const auto& a, const auto& b) { return a.value < b.value; });
      });

  ExpectSortIsCorrect(
      [](auto& structs) {
        orbit_base::sort(std::begin(structs), std::end(structs), kProjection, std::greater<>{});
      },
      [](auto& structs) {
        std::sort(std::begin(structs), std::end(structs),
                  [](const auto& a, const auto& b) { return a.value > b.value; });
      });

  ExpectSortIsCorrect(
      [](auto& structs) {
        orbit_base::stable_sort(std::begin(structs), std::end(structs), kProjection);
      },
      [](auto& structs) {
        std::stable_sort(std::begin(structs), std::end(structs),
                         [](const auto& a, const auto& b) { return a.value < b.value; });
      });

  ExpectSortIsCorrect(
      [](auto& structs) {
        orbit_base::stable_sort(std::begin(structs), std::end(structs), kProjection,
                                std::greater<>{});
      },
      [](auto& structs) {
        std::stable_sort(std::begin(structs), std::end(structs),
                         [](const auto& a, const auto& b) { return a.value > b.value; });
      });
}

}  // namespace orbit_base