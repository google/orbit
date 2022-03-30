// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <gtest/gtest.h>

#include <memory>

#include "ClientData/FunctionInfo.h"

namespace orbit_client_data {

TEST(FunctionInfoSet, EqualFunctions) {
  FunctionInfo left{"foo()", "/path/to/module", "buildid", 12, 16};
  FunctionInfo right{"foo()", "/path/to/module", "buildid", 12, 16};

  EXPECT_TRUE(left == right);
  EXPECT_TRUE(right == left);
  EXPECT_EQ(left, right);
}

TEST(FunctionInfoSet, DifferentName) {
  FunctionInfo left{"foo()", "/path/to/module", "buildid", 12, 16};
  FunctionInfo right{"bar()", "/path/to/module", "buildid", 12, 16};

  EXPECT_EQ(left, right);
}

TEST(FunctionInfoSet, DifferentModulePath) {
  FunctionInfo left{"foo()", "/path/to/module", "buildid", 12, 16};
  FunctionInfo right{"foo()", "/path/to/other", "buildid", 12, 16};

  EXPECT_NE(left, right);
}

TEST(FunctionInfoSet, DifferentBuildId) {
  FunctionInfo left{"foo()", "/path/to/module", "buildid", 12, 16};
  FunctionInfo right{"foo()", "/path/to/module", "anotherbuildid", 12, 16};

  EXPECT_NE(left, right);
}

TEST(FunctionInfoSet, DifferentAddress) {
  FunctionInfo left{"foo()", "/path/to/module", "buildid", 12, 16};
  FunctionInfo right{"foo()", "/path/to/module", "buildid", 14, 16};

  EXPECT_NE(left, right);
}

TEST(FunctionInfoSet, DifferentSize) {
  FunctionInfo left{"foo()", "/path/to/module", "buildid", 12, 16};
  FunctionInfo right{"foo()", "/path/to/module", "buildid", 12, 15};

  EXPECT_EQ(left, right);
}

TEST(FunctionInfoSet, Insertion) {
  FunctionInfo function{"foo()", "/path/to/module", "buildid", 12, 16};

  absl::flat_hash_set<FunctionInfo> functions;
  EXPECT_FALSE(functions.contains(function));
  functions.insert(function);
  EXPECT_TRUE(functions.contains(function));
  EXPECT_EQ(functions.size(), 1);

  FunctionInfo other{"bar()", "/path/to/module", "buildid", 512, 14};
  EXPECT_FALSE(functions.contains(other));
}

TEST(FunctionInfoSet, Deletion) {
  FunctionInfo function{"foo()", "/path/to/module", "buildid", 12, 16};
  ;

  absl::flat_hash_set<FunctionInfo> functions;
  functions.insert(function);
  EXPECT_TRUE(functions.contains(function));
  EXPECT_EQ(functions.size(), 1);

  FunctionInfo other{"bar()", "/path/to/module", "buildid", 512, 14};
  ;
  EXPECT_FALSE(functions.contains(other));
  functions.erase(other);
  EXPECT_FALSE(functions.contains(other));
  EXPECT_EQ(functions.size(), 1);

  functions.erase(function);
  EXPECT_FALSE(functions.contains(function));
  EXPECT_EQ(functions.size(), 0);
}

}  // namespace orbit_client_data
