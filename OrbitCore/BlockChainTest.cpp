// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <BlockChain.h>
#include <gtest/gtest.h>

#include <string>

namespace {

class CopyableType {
 public:
  CopyableType() = default;
  explicit CopyableType(std::string value) : value_{std::move(value)} {}

  CopyableType(const CopyableType&) = default;
  CopyableType& operator=(const CopyableType&) = default;

  void set_value(std::string value) { value_ = std::move(value); }
  [[nodiscard]] const std::string& value() const { return value_; }

 private:
  std::string value_;
};

class MovableType {
 public:
  MovableType() = default;
  explicit MovableType(std::string value) : value_{std::move(value)} {}

  MovableType(const MovableType&) = delete;
  MovableType& operator=(const MovableType&) = delete;

  MovableType(MovableType&&) = default;
  MovableType& operator=(MovableType&&) = default;

  [[nodiscard]] const std::string& value() const { return value_; }

 private:
  std::string value_;
};

};  // namespace

TEST(BlockChain, AddCopyableTypes) {
  CopyableType v1("hello world");
  CopyableType v2("or not");

  BlockChain<CopyableType, 1024> chain;
  EXPECT_EQ(chain.size(), 0);
  chain.push_back(v1);
  chain.push_back(v2);
  EXPECT_EQ(chain.size(), 2);

  v1.set_value("new v1");
  v2.set_value("new v2");

  EXPECT_EQ(chain.root()->data()[0].value(), "hello world");
  EXPECT_EQ(chain.root()->data()[1].value(), "or not");

  // Multi-block test
  for (int i = 0; i < 2000; ++i) {
    chain.push_back(v1);
  }
  EXPECT_EQ(chain.size(), 2002);
}

TEST(BlockChain, Clear) {
  const std::string v1 = "hello world";
  const std::string v2 = "or not";

  BlockChain<std::string, 1024> chain;
  chain.push_back(v1);
  EXPECT_GT(chain.size(), 0);
  chain.clear();
  EXPECT_EQ(chain.size(), 0);

  chain.push_back(v2);
  EXPECT_GT(chain.size(), 0);
  EXPECT_EQ(chain.root()->data()[0], v2);

  // Multi-block test
  for (int i = 0; i < 2000; ++i) {
    chain.push_back(v1);
  }
  chain.clear();
  EXPECT_EQ(chain.size(), 0);
}

TEST(BlockChain, ElementIteration) {
  constexpr const int v1 = 5;
  constexpr const int v2 = 10;
  constexpr const int v3 = 15;

  BlockChain<int, 1024> chain;

  chain.push_back(v1);
  chain.push_back(v2);
  chain.push_back(v3);

  // Note that only the "++it" operator is supported
  auto it = chain.begin();
  EXPECT_EQ(*it, v1);
  ++it;
  EXPECT_EQ(*it, v2);
  ++it;
  EXPECT_EQ(*it, v3);
  ++it;
  // ... and also only !=, not ==
  EXPECT_FALSE(it != chain.end());

  // Test the complete "typical pattern"
  int it_count = 0;
  for (it = chain.begin(); it != chain.end(); ++it) {
    ++it_count;
  }
  EXPECT_EQ(it_count, 3);

  // Multi-block test
  chain.clear();
  for (int i = 0; i < 2000; ++i) {
    chain.push_back(i);
  }
  it_count = 0;

  for (it = chain.begin(); it != chain.end(); ++it) {
    EXPECT_EQ(*it, it_count);
    ++it_count;
  }

  auto it_begin = chain.begin();
  it = chain.begin();
  ++it;
  while (it != chain.end()) {
    EXPECT_TRUE(it != it_begin);
    ++it;
  }

  EXPECT_EQ(it_count, 2000);
}

TEST(BlockChain, EmptyIteration) {
  BlockChain<std::string, 1024> chain;
  auto it = chain.begin();
  ASSERT_FALSE(it != chain.end());
}

TEST(BlockChain, AddCopyableTypesN) {
  const std::string v1 = "hello world";
  BlockChain<std::string, 1024> chain;
  chain.push_back_n(v1, 2000);
  EXPECT_EQ(chain.size(), 2000);
  for (auto& it : chain) {
    EXPECT_EQ(it, v1);
  }
}

// "Reset" works like "clear", except that it does not actually free
// any memory - it keeps the block's memory, just setting their
// size to 0
TEST(BlockChain, Reset) {
  BlockChain<int, 1024> chain;
  chain.push_back_n(5, 1024 * 3);
  EXPECT_GT(chain.size(), 0);
  const Block<int, 1024>* blockPtr[] = {chain.root(), nullptr, nullptr};
  blockPtr[1] = blockPtr[0]->next();
  blockPtr[2] = blockPtr[1]->next();

  // Tests below rely quite a lot on the internals of BlockChain, but this
  // seems the easiest way to actually test re-usage of the block pointers
  chain.Reset();
  EXPECT_EQ(chain.size(), 0);
  EXPECT_EQ(blockPtr[0]->size(), 0);
  EXPECT_EQ(blockPtr[1]->size(), 0);
  EXPECT_EQ(blockPtr[2]->size(), 0);

  chain.push_back_n(10, 1024);
  EXPECT_GT(chain.size(), 0);
  EXPECT_EQ(chain.root()->data()[0], 10);
  EXPECT_EQ(chain.root(), blockPtr[0]);
  EXPECT_EQ(chain.root()->next(), blockPtr[1]);
  EXPECT_EQ(blockPtr[1]->size(), 0);

  chain.push_back_n(10, 1024);
  EXPECT_EQ(chain.root()->next(), blockPtr[1]);
  EXPECT_EQ(blockPtr[1]->size(), 1024);
  EXPECT_EQ(blockPtr[2]->size(), 0);

  chain.push_back_n(10, 1024);
  EXPECT_EQ(chain.root()->next()->next(), blockPtr[2]);
  EXPECT_EQ(blockPtr[2]->size(), 1024);
}

TEST(BlockChain, MovableType) {
  BlockChain<MovableType, 1024> chain;
  EXPECT_EQ(chain.size(), 0);
  chain.push_back(MovableType("v1"));
  chain.push_back(MovableType("v2"));
  EXPECT_EQ(chain.size(), 2);

  EXPECT_EQ(chain.root()->data()[0].value(), "v1");
  EXPECT_EQ(chain.root()->data()[1].value(), "v2");
}
