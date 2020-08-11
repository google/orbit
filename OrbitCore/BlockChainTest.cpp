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

  EXPECT_EQ(chain.SlowAt(0)->value(), "hello world");
  EXPECT_EQ(chain.SlowAt(1)->value(), "or not");

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
  EXPECT_EQ(*chain.SlowAt(0), v2);

  // Multi-block test
  for (int i = 0; i < 2000; ++i) {
    chain.push_back(v1);
  }
  chain.clear();
  EXPECT_EQ(chain.size(), 0);
}

TEST(BlockChain, RetrieveCopyableTypes) {
  const std::string v1 = "hello world";
  const std::string v2 = "or not";
  
  BlockChain<std::string, 1024> chain;
  chain.push_back(v1);
  chain.push_back(v2);
  EXPECT_EQ(*chain.SlowAt(0), v1);
  EXPECT_EQ(*chain.SlowAt(1), v2);

  // Expect those types to be copied
  EXPECT_NE(chain.SlowAt(0), &v1);
  EXPECT_NE(chain.SlowAt(1), &v2);

  // Multi-block test
  chain.clear();
  for (int i = 0; i < 2000; ++i) {
    chain.push_back(i % 2 == 0 ? v1 : v2);
  }
  for (int i = 0; i < 2000; ++i) {
    EXPECT_EQ(*chain.SlowAt(i), i % 2 == 0 ? v1 : v2);
  }
}

TEST(BlockChain, ElementIteration) {
  constexpr const int v1 = 5;
  constexpr const int v2 = 10;
  constexpr const int v3 = 15;

  BlockChain<int, 1024> chain;
  chain.push_back(v1);
  chain.push_back(v2);
  chain.push_back(v3);

  // The original element can't be found - BlockChain manages copies!
  EXPECT_EQ(chain.GetElementAfter(&v1), nullptr);

  int* el = chain.GetElementAfter(chain.SlowAt(0));
  EXPECT_NE(el, nullptr);
  EXPECT_EQ(*el, v2);

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
  EXPECT_EQ(it_count, 2000);
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
  Block<int, 1024>* blockPtr[] = {chain.GetBlockContaining(chain.SlowAt(0)),
                                  nullptr, nullptr};
  blockPtr[1] = blockPtr[0]->m_Next;
  blockPtr[2] = blockPtr[1]->m_Next;

  // Tests below rely quite a lot on the internals of BlockChain, but this
  // seems the easiest way to actually test re-usage of the block pointers
  chain.Reset();
  EXPECT_EQ(chain.size(), 0);
  EXPECT_EQ(blockPtr[0]->m_Size, 0);
  EXPECT_EQ(blockPtr[1]->m_Size, 0);
  EXPECT_EQ(blockPtr[2]->m_Size, 0);

  chain.push_back_n(10, 1024);
  EXPECT_GT(chain.size(), 0);
  EXPECT_EQ(*chain.SlowAt(0), 10);
  EXPECT_EQ(chain.m_Root, blockPtr[0]);
  EXPECT_EQ(chain.m_Root->m_Next, blockPtr[1]);
  EXPECT_EQ(blockPtr[1]->m_Size, 0);

  chain.push_back_n(10, 1024);
  EXPECT_EQ(chain.m_Root->m_Next, blockPtr[1]);
  EXPECT_EQ(blockPtr[1]->m_Size, 1024);
  EXPECT_EQ(blockPtr[2]->m_Size, 0);

  chain.push_back_n(10, 1024);
  EXPECT_EQ(chain.m_Root->m_Next->m_Next, blockPtr[2]);
  EXPECT_EQ(blockPtr[2]->m_Size, 1024);
}

TEST(BlockChain, keep) {
  BlockChain<int, 1024> chain;
  for (int i = 0; i < 100; ++i) {
    chain.push_back(i);
  }
  EXPECT_EQ(chain.size(), 100);
  chain.keep(10);
  // keep() keeps the LAST n elements, but always keeps at least 2 blocks
  EXPECT_EQ(chain.size(), 100);

  chain.clear();
  for (int i = 0; i < 2000; ++i) {
    chain.push_back(i);
  }
  EXPECT_EQ(chain.size(), 2000);
  chain.keep(10);
  EXPECT_EQ(chain.size(), 2000 - 1024);
  for (size_t i = 0; i < chain.size(); ++i) {
    EXPECT_EQ(*chain.SlowAt(i), i + 1024);
  }
}

TEST(BlockChain, MovableType) {
  BlockChain<MovableType, 1024> chain;
  EXPECT_EQ(chain.size(), 0);
  chain.push_back(MovableType("v1"));
  chain.push_back(MovableType("v2"));
  EXPECT_EQ(chain.size(), 2);

  EXPECT_EQ(chain.SlowAt(0)->value(), "v1");
  EXPECT_EQ(chain.SlowAt(1)->value(), "v2");
}
