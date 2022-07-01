// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_SORT_H_
#define ORBIT_BASE_SORT_H_

#include <algorithm>
#include <execution>
#include <functional>
#include <type_traits>

namespace orbit_base_internal {

template <typename Comparator, typename Projection>
static auto MakeComparator(Comparator comparator, Projection projection) {
  return [comparator, projection](const auto& a, const auto& b) {
    return comparator(projection(a), projection(b));
  };
}

}  // namespace orbit_base_internal

namespace orbit_base {

template <typename ExecutionPolicy, typename RandomIt, typename Projection,
          typename Comparator = std::less<>,
          typename = std::enable_if<
              std::is_invocable_v<Projection, typename std::iterator_traits<RandomIt>::value_type>>>
void sort(ExecutionPolicy&& policy, RandomIt first, RandomIt last, Projection projection = {},
          Comparator comparator = {}) {
  std::sort(std::forward(policy), first, last,
            orbit_base_internal::MakeComparator(comparator, projection));
}

template <typename RandomIt, typename Projection, typename Comparator = std::less<>,
          typename = std::enable_if<
              std::is_invocable_v<Projection, typename std::iterator_traits<RandomIt>::value_type>>>
void sort(RandomIt first, RandomIt last, Projection projection = {}, Comparator comparator = {}) {
  std::sort(first, last, orbit_base_internal::MakeComparator(comparator, projection));
}

template <typename ExecutionPolicy, typename RandomIt, typename Projection,
          typename Comparator = std::less<>,
          typename = std::enable_if<
              std::is_invocable_v<Projection, typename std::iterator_traits<RandomIt>::value_type>>>
void stable_sort(ExecutionPolicy&& policy, RandomIt first, RandomIt last,
                 Projection projection = {}, Comparator comparator = {}) {
  std::stable_sort(std::forward(policy), first, last,
                   orbit_base_internal::MakeComparator(comparator, projection));
}

template <typename RandomIt, typename Projection, typename Comparator = std::less<>,
          typename = std::enable_if<
              std::is_invocable_v<Projection, typename std::iterator_traits<RandomIt>::value_type>>>
void stable_sort(RandomIt first, RandomIt last, Projection projection = {},
                 Comparator comparator = {}) {
  std::sort(first, last, orbit_base_internal::MakeComparator(comparator, projection));
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_SORT_H_
