#include <gtest/gtest.h>

#include "StringManager.h"

TEST(StringManager, Contains) {
  StringManager string_manager;
  string_manager.AddIfNotPresent(0, "test1");
  EXPECT_TRUE(string_manager.Contains(0));
  EXPECT_FALSE(string_manager.Contains(1));
}

TEST(StringManager, AddIfNotPresent) {
  StringManager string_manager;
  string_manager.AddIfNotPresent(0, "test1");
  string_manager.AddIfNotPresent(0, "test2");
  EXPECT_EQ("test1", string_manager.Get(0).value_or(""));
  EXPECT_TRUE(string_manager.Contains(0));
}

TEST(StringManager, Get) {
  StringManager string_manager;
  string_manager.AddIfNotPresent(0, "test1");
  EXPECT_EQ("test1", string_manager.Get(0).value_or(""));
  EXPECT_FALSE(string_manager.Get(1).has_value());
}
