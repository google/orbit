// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <gtest/gtest.h>

#include <string>

#include "ClientData/FunctionInfo.h"

namespace orbit_client_data {

TEST(FunctionInfo, EqualFunctions) {
  FunctionInfo left{"/path/to/module", "buildid", /*address=*/12,
                    /*size=*/16,       "foo()",   /*is_hotpatchable=*/false};
  FunctionInfo right{"/path/to/module", "buildid", /*address=*/12,
                     /*size=*/16,       "foo()",   /*is_hotpatchable=*/false};

  EXPECT_TRUE(left == right);
  EXPECT_TRUE(right == left);
  EXPECT_EQ(left, right);
}

TEST(FunctionInfo, DifferentName) {
  FunctionInfo left{"/path/to/module", "buildid", /*address=*/12,
                    /*size=*/16,       "foo()",   /*is_hotpatchable=*/false};
  FunctionInfo right{"/path/to/module", "buildid", 12, 16, "bar()", false};

  EXPECT_EQ(left, right);
}

TEST(FunctionInfo, DifferentModulePath) {
  FunctionInfo left{"/path/to/module", "buildid", /*address=*/12,
                    /*size=*/16,       "foo()",   /*is_hotpatchable=*/false};
  FunctionInfo right{"/path/to/other", "buildid", /*address=*/12,
                     /*size=*/16,      "foo()",   /*is_hotpatchable=*/false};

  EXPECT_NE(left, right);
}

TEST(FunctionInfo, DifferentBuildId) {
  FunctionInfo left{"/path/to/module", "buildid", /*address=*/12,
                    /*size=*/16,       "foo()",   /*is_hotpatchable=*/false};
  FunctionInfo right{"/path/to/module", "anotherbuildid", /*address=*/12,
                     /*size=*/16,       "foo()",          /*is_hotpatchable=*/false};

  EXPECT_NE(left, right);
}

TEST(FunctionInfo, DifferentAddress) {
  FunctionInfo left{"/path/to/module", "buildid", /*address=*/12,
                    /*size=*/16,       "foo()",   /*is_hotpatchable=*/false};
  FunctionInfo right{"/path/to/module", "buildid", 14, 16, "foo()", false};

  EXPECT_NE(left, right);
}

TEST(FunctionInfo, DifferentSize) {
  FunctionInfo left{"/path/to/module", "buildid", /*address=*/12,
                    /*size=*/16,       "foo()",   /*is_hotpatchable=*/false};
  FunctionInfo right{"/path/to/module", "buildid", 12, 15, "foo()", false};

  EXPECT_EQ(left, right);
}

TEST(FunctionInfo, InsertionIntoSet) {
  FunctionInfo function{"/path/to/module", "buildid", /*address=*/12,
                        /*size=*/16,       "foo()",   /*is_hotpatchable=*/false};

  absl::flat_hash_set<FunctionInfo> functions;
  EXPECT_FALSE(functions.contains(function));
  functions.insert(function);
  EXPECT_TRUE(functions.contains(function));
  EXPECT_EQ(functions.size(), 1);

  FunctionInfo other{"/path/to/module", "buildid", 512, 14, "bar()", false};
  EXPECT_FALSE(functions.contains(other));
}

TEST(FunctionInfo, DeletionFromSet) {
  FunctionInfo function{"/path/to/module", "buildid", /*address=*/12,
                        /*size=*/16,       "foo()",   /*is_hotpatchable=*/false};

  absl::flat_hash_set<FunctionInfo> functions;
  functions.insert(function);
  EXPECT_TRUE(functions.contains(function));
  EXPECT_EQ(functions.size(), 1);

  FunctionInfo other{"/path/to/module", "buildid", 512, 14, "bar()", false};
  EXPECT_FALSE(functions.contains(other));
  functions.erase(other);
  EXPECT_FALSE(functions.contains(other));
  EXPECT_EQ(functions.size(), 1);

  functions.erase(function);
  EXPECT_FALSE(functions.contains(function));
  EXPECT_EQ(functions.size(), 0);
}

}  // namespace orbit_client_data
