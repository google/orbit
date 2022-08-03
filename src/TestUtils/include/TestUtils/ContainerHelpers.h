// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TEST_UTILS_CONTAINER_HELPERS_H_
#define TEST_UTILS_CONTAINER_HELPERS_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <algorithm>
#include <iterator>
#include <type_traits>

namespace orbit_test_utils {

// A helper function that takes two collection (possibly, of different type) and returns a map whose
// entries are the element-wise pairs of the elements of the two input collections.
// If the sizes of the collections differ, the excessive elements are ignored.
template <typename Container, typename AnotherContainer>
[[nodiscard]] static auto MakeMap(Container&& keys, AnotherContainer&& values) {
  using K = typename std::decay_t<Container>::value_type;
  using V = typename std::decay_t<AnotherContainer>::value_type;
  using std::begin;
  using std::end;

  absl::flat_hash_map<K, V> result;
  std::transform(begin(keys), end(keys), begin(values), std::inserter(result, std::begin(result)),
                 [](const K& k, const V& v) { return std::make_pair(k, v); });
  return result;
}

// A helper function that takes two collection (possibly, of different type) s. t. the elements of
// `b` are convertible to the type of elements of `a`. An `std::vector` containing the elements
// present in both collections (up to the said conversion) is returned.
template <typename Container, typename AnotherContainer,
          typename E = typename std::decay_t<Container>::value_type,
          typename OtherE = typename std::decay_t<AnotherContainer>::value_type,
          typename = std::enable_if_t<std::is_convertible_v<OtherE, E>>>
[[nodiscard]] static std::vector<E> Commons(Container&& a, AnotherContainer&& b) {
  using std::begin;
  using std::end;

  absl::flat_hash_set<E> a_set(begin(a), end(a));
  std::vector<E> result;
  std::copy_if(begin(b), end(b), std::back_inserter(result),
               [&a_set](const E& element) { return a_set.contains(element); });
  return result;
}

}  // namespace orbit_test_utils

#endif  // TEST_UTILS_CONTAINER_HELPERS_H_
