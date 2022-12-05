// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TEST_UTILS_SAVE_RANGE_FROM_ARG_H_
#define TEST_UTILS_SAVE_RANGE_FROM_ARG_H_

#include <stddef.h>

#include <iterator>
#include <tuple>
#include <type_traits>

namespace orbit_test_utils {

// SaveRangeFromArg is a GMock action and behaves similar to testing::SaveArg, but expects the
// argument to be a range (has cbegin and cend iterators) and copies individual elements into a
// newly constructed object of type decltype(*Ptr). We expect that *Ptr has a constructor that takes
// two iterators to construct from a range.
//
// This is useful if the argument is an absl::Span and we want to store the elements in a
// std::vector (or any other container that can be constructed from a range - like a std::set for
// example).
//
// The first template argument `k` indicates the we want to save the k-th argument. `k` is
// 0-indexed.
//
// Example:
// std::vector<int> vec{};
// EXPECT_CALL(obj, MyFunction).WillOnce(SaveRangeFromArg<2>(&vec));
//
template <size_t k, typename Ptr>
[[nodiscard]] auto SaveRangeFromArg(Ptr dest) {
  return [dest](const auto&... args) {
    const auto& kth_arg = std::get<k>(std::tie(args...));
    using std::cbegin;
    using std::cend;
    // Container::assign would be nicer here, but this member function doesn't exist for all kinds
    // of standard containers (Exists for std::vector but not for std::set for example).
    *dest = std::decay_t<decltype(*dest)>{cbegin(kth_arg), cend(kth_arg)};
  };
}

}  // namespace orbit_test_utils

#endif  // TEST_UTILS_SAVE_RANGE_FROM_ARG_H_