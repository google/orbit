// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_SORT_H_
#define ORBIT_BASE_SORT_H_

#include <algorithm>
#include <functional>
#include <type_traits>
#include <utility>

namespace orbit_base_internal {

template <typename Comparator, typename Projection>
static auto MakeComparator(Comparator&& comparator, Projection&& projection) {
  return [comparator = std::forward<Comparator>(comparator),
          projection = std::forward<Projection>(projection)](const auto& a, const auto& b) {
    return std::invoke(comparator, std::invoke(projection, a), std::invoke(projection, b));
  };
}

}  // namespace orbit_base_internal

namespace orbit_base {

// Often, custom comparators passed to std::sort take form
// `[const auto& a, const auto& b] {return projection(a) < projection(b)}`. To that end we introduce
// this function, that takes `projection` explicitly and relieves the caller from the burden of
// manual instantiation of the comparator comparing `a` and `b`. The function also takes comparator
// that define an order over the values returned by `projection`.
//
// This will sort structs in the order of decreasing values of field `value`.
// ```
// orbit_base::sort(std::begin(structs), std::end(structs),
//             [](const auto & s) { return s.value; }, std::greater<>{});
// ```
template <typename RandomIt, typename Projection, typename Comparator = std::less<>,
          typename = std::enable_if_t<
              std::is_invocable_v<Projection, typename std::iterator_traits<RandomIt>::value_type>>>
void sort(RandomIt first, RandomIt last, Projection projection = {}, Comparator comparator = {}) {
  std::sort(first, last,
            orbit_base_internal::MakeComparator(std::move(comparator), std::move(projection)));
}

// Stable counterpart of orbit_base::sort
template <typename RandomIt, typename Projection, typename Comparator = std::less<>,
          typename = std::enable_if_t<
              std::is_invocable_v<Projection, typename std::iterator_traits<RandomIt>::value_type>>>
void stable_sort(RandomIt first, RandomIt last, Projection projection = {},
                 Comparator comparator = {}) {
  std::stable_sort(
      first, last,
      orbit_base_internal::MakeComparator(std::move(comparator), std::move(projection)));
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_SORT_H_
