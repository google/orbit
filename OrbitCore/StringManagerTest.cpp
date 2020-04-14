#include <gtest/gtest.h>

#include "StringManager.h"

TEST(StringManager, Exists) {
  StringManager string_manager;
  string_manager.Add(0, "test1");
  EXPECT_TRUE(string_manager.Exists(0));
  EXPECT_FALSE(string_manager.Exists(1));
}

TEST(StringManager, Add) {
  StringManager string_manager;
  string_manager.Add(0, "test1");
  string_manager.Add(0, "test2");
  EXPECT_EQ("test1", string_manager.Get(0).value_or(""));
  EXPECT_TRUE(string_manager.Exists(0));
}

TEST(StringManager, Get) {
  StringManager string_manager;
  string_manager.Add(0, "test1");
  EXPECT_EQ("test1", string_manager.Get(0).value_or(""));
  EXPECT_FALSE(string_manager.Get(1).has_value());
}
