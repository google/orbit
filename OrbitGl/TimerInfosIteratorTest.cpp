// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "TextBox.h"
#include "TimerChain.h"
#include "TimerInfosIterator.h"
#include "capture_data.pb.h"

using orbit_client_protos::TimerInfo;

TEST(TimerInfosIterator, Access) {
  std::vector<std::shared_ptr<TimerChain>> chains;
  std::shared_ptr<TimerChain> chain = std::make_shared<TimerChain>();
  TextBox box;
  TimerInfo timer;
  timer.set_function_address(1);
  // Need to set the end, as SetTimerInfo will refuse that timer otherwise.
  timer.set_end(1);
  box.SetTimerInfo(timer);
  chain->push_back(box);
  chains.push_back(chain);

  // Just validate setting worked as expected
  EXPECT_EQ(1, timer.function_address());
  EXPECT_EQ(1, box.GetTimerInfo().function_address());

  // Now create an iterator and test to access it
  TimerInfosIterator it(chains.begin(), chains.end());
  EXPECT_EQ(1, it->function_address());
  EXPECT_EQ(1, (*it).function_address());
}

TEST(TimerInfosIterator, Copy) {
  std::vector<std::shared_ptr<TimerChain>> chains;
  std::shared_ptr<TimerChain> chain = std::make_shared<TimerChain>();
  TextBox box;
  TimerInfo timer;
  timer.set_function_address(1);
  // Need to set the end, as SetTimerInfo will refuse that timer otherwise.
  timer.set_end(1);
  box.SetTimerInfo(timer);
  chain->push_back(box);
  chains.push_back(chain);

  // Now create an iterator and test to access it
  TimerInfosIterator it(chains.begin(), chains.end());
  EXPECT_EQ(1, it->function_address());

  // Now create a Copy
  TimerInfosIterator it_copy1 = it;
  EXPECT_EQ(1, it_copy1->function_address());

  // Increase the original and check that the copy does not modify
  ++it;
  EXPECT_EQ(1, it_copy1->function_address());

  // Create a copy using the copy-constructor
  TimerInfosIterator it_copy2(it_copy1);
  EXPECT_EQ(1, it_copy2->function_address());

  // Increase the original and check that the copy does not modify
  ++it_copy1;
  EXPECT_EQ(1, it_copy2->function_address());
}

TEST(TimerInfosIterator, Move) {
  std::vector<std::shared_ptr<TimerChain>> chains;
  std::shared_ptr<TimerChain> chain = std::make_shared<TimerChain>();
  TextBox box;
  TimerInfo timer;
  timer.set_function_address(1);
  // Need to set the end, as SetTimerInfo will refuse that timer otherwise.
  timer.set_end(1);
  box.SetTimerInfo(timer);
  chain->push_back(box);
  chains.push_back(chain);

  // Now create an iterator and test to access it
  TimerInfosIterator it(chains.begin(), chains.end());
  EXPECT_EQ(1, it->function_address());

  // Now create a Copy
  TimerInfosIterator it_copy1 = std::move(it);
  EXPECT_EQ(1, it_copy1->function_address());

  // Create a copy using the copy-constructor
  TimerInfosIterator it_copy2(std::move(it_copy1));
  EXPECT_EQ(1, it_copy2->function_address());
}

TEST(TimerInfosIterator, Equality) {
  std::vector<std::shared_ptr<TimerChain>> chains;
  std::shared_ptr<TimerChain> chain = std::make_shared<TimerChain>();
  TextBox box;
  TimerInfo timer;
  timer.set_function_address(1);
  timer.set_end(1);
  box.SetTimerInfo(timer);
  chain->push_back(box);
  chains.push_back(chain);

  // Now create an iterators and test equality
  TimerInfosIterator it1(chains.begin(), chains.end());
  TimerInfosIterator it2(chains.begin(), chains.end());
  TimerInfosIterator it_end(chains.end(), chains.end());

  EXPECT_TRUE(it1 == it2);
  EXPECT_FALSE(it1 != it2);

  EXPECT_FALSE(it1 == it_end);
  EXPECT_TRUE(it1 != it_end);

  ++it1;
  EXPECT_TRUE(it1 == it_end);
  EXPECT_FALSE(it1 != it_end);
  EXPECT_FALSE(it1 == it2);
  EXPECT_TRUE(it1 != it2);
}

TEST(TimerInfosIterator, ForEachEmpty) {
  std::vector<std::shared_ptr<TimerChain>> chains;

  TimerInfosIterator it_begin(chains.begin(), chains.end());
  TimerInfosIterator it_end(chains.begin(), chains.end());

  std::vector<uint64_t> result;
  for (auto it = it_begin; it != it_end; ++it) {
    result.push_back(it->function_address());
  }
  EXPECT_THAT(result, testing::ElementsAre());
}

TEST(TimerInfosIterator, ForEachLarge) {
  std::vector<std::shared_ptr<TimerChain>> chains;
  std::vector<uint64_t> expected;

  size_t count = 0;
  size_t max_timers = 32;
  // Add twelve chains, with the size 2⁴, 2⁵, 2⁶, ...
  //  such that there are blocks inside the chains that are full (1024 elements),
  //  as well as blocks that are not full.
  for (size_t chain_count = 0; chain_count < 12; ++chain_count) {
    std::shared_ptr<TimerChain> chain = std::make_shared<TimerChain>();
    for (size_t box_count = 0; box_count < max_timers; ++box_count) {
      TextBox box;
      TimerInfo timer;
      timer.set_function_address(count);
      timer.set_start(count);
      timer.set_end(count + 1);
      box.SetTimerInfo(timer);
      chain->push_back(box);
      expected.push_back(count);
      ++count;
    }
    max_timers *= 2;
    chains.push_back(chain);
  }

  TimerInfosIterator it_begin(chains.begin(), chains.end());
  TimerInfosIterator it_end(chains.end(), chains.end());

  std::vector<uint64_t> result;
  for (auto it = it_begin; it != it_end; ++it) {
    result.push_back(it->function_address());
  }
  EXPECT_THAT(result, testing::ElementsAreArray(expected));
}