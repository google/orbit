// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "StringManager/StringManager.h"

namespace orbit_string_manager {

TEST(StringManager, AddIfNotPresent) {
  StringManager string_manager;
  EXPECT_TRUE(string_manager.AddIfNotPresent(0, "test1"));
  EXPECT_FALSE(string_manager.AddIfNotPresent(0, "test2"));

  EXPECT_EQ("test1", string_manager.Get(0).value_or("no value"));
  EXPECT_TRUE(string_manager.Contains(0));
}

TEST(StringManager, AddOrReplace) {
  StringManager string_manager;
  EXPECT_TRUE(string_manager.AddIfNotPresent(0, "test1"));
  EXPECT_FALSE(string_manager.AddOrReplace(0, "test2"));
  EXPECT_TRUE(string_manager.AddOrReplace(1, "test3"));
  EXPECT_FALSE(string_manager.AddOrReplace(1, "test4"));

  EXPECT_TRUE(string_manager.Contains(0));
  EXPECT_EQ("test2", string_manager.Get(0).value_or("no value"));
  EXPECT_TRUE(string_manager.Contains(1));
  EXPECT_EQ("test4", string_manager.Get(1).value_or("no value"));
}

TEST(StringManager, Contains) {
  StringManager string_manager;
  string_manager.AddIfNotPresent(0, "test1");

  EXPECT_TRUE(string_manager.Contains(0));
  EXPECT_FALSE(string_manager.Contains(1));
}

TEST(StringManager, Get) {
  StringManager string_manager;
  string_manager.AddIfNotPresent(0, "test1");

  EXPECT_EQ("test1", string_manager.Get(0).value_or("no value"));
  EXPECT_FALSE(string_manager.Get(1).has_value());
}

TEST(StringManager, Clear) {
  StringManager string_manager;
  string_manager.AddIfNotPresent(0, "test1");
  string_manager.AddIfNotPresent(1, "test2");
  string_manager.Clear();

  EXPECT_FALSE(string_manager.Contains(0));
  EXPECT_FALSE(string_manager.Contains(1));
}

}  // namespace orbit_string_manager
