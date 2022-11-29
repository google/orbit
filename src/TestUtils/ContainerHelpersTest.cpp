// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "TestUtils/ContainerHelpers.h"

namespace orbit_test_utils {

constexpr auto kKeys = {1, 2, 3};
constexpr auto kValues = {'a', 'b', 'c', 'd'};
constexpr std::array<char, kValues.size()> kValueArray{'a', 'b', 'c', 'd'};
const absl::flat_hash_map<int, char> kExpectedMap = {{1, 'a'}, {2, 'b'}, {3, 'c'}};

using testing::IsEmpty;
using testing::UnorderedElementsAreArray;

TEST(ContainerHelpersTest, MakeMapIsCorrect) {
  {
    auto keys = kKeys;
    auto values = kValues;
    EXPECT_THAT(MakeMap(std::move(keys), std::move(values)),
                UnorderedElementsAreArray(kExpectedMap));
  }

  {
    std::vector<int> keys = kKeys;
    EXPECT_THAT(MakeMap(std::move(keys), kValueArray), UnorderedElementsAreArray(kExpectedMap));
  }

  {
    const std::vector<int> keys = kKeys;
    std::vector<char> values = kValues;
    EXPECT_THAT(MakeMap(keys, std::move(values)), UnorderedElementsAreArray(kExpectedMap));
  }
}

constexpr auto kFirstCollection = {"foo()", "bar()", "biz()"};
constexpr auto kOtherCollection = {"foo()", "bar()", "fiz()"};
constexpr std::array<std::string_view, 2> kCommons = {"foo()", "bar()"};

TEST(ContainerHelpersTest, CommonsIsCorrect) {
  EXPECT_THAT(Commons(kFirstCollection, kOtherCollection), UnorderedElementsAreArray(kCommons));

  EXPECT_THAT(Commons(std::initializer_list<std::string>{}, kOtherCollection), IsEmpty());

  const std::vector<const char*> first_collection = kFirstCollection;
  EXPECT_THAT(Commons(first_collection, kOtherCollection), UnorderedElementsAreArray(kCommons));
}

}  // namespace orbit_test_utils