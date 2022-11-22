// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

#include "ClientData/TimerChain.h"
#include "ClientProtos/capture_data.pb.h"
#include "OrbitGl/TimerInfosIterator.h"

using orbit_client_protos::TimerInfo;

TEST(TimerInfosIterator, Access) {
  std::vector<std::shared_ptr<orbit_client_data::TimerChain>> chains;
  std::shared_ptr<orbit_client_data::TimerChain> chain =
      std::make_shared<orbit_client_data::TimerChain>();
  TimerInfo timer;
  timer.set_function_id(1);
  chain->emplace_back(timer);
  chains.emplace_back(chain);

  // Just validate setting worked as expected
  EXPECT_EQ(1, timer.function_id());

  // Now create an iterator and test to access it
  TimerInfosIterator it(chains.begin(), chains.end());
  EXPECT_EQ(1, it->function_id());
  EXPECT_EQ(1, (*it).function_id());
}

TEST(TimerInfosIterator, Copy) {
  std::vector<std::shared_ptr<orbit_client_data::TimerChain>> chains;
  std::shared_ptr<orbit_client_data::TimerChain> chain =
      std::make_shared<orbit_client_data::TimerChain>();
  TimerInfo timer;
  timer.set_function_id(1);
  chain->emplace_back(timer);
  chains.emplace_back(chain);

  // Now create an iterator and test to access it
  TimerInfosIterator it(chains.begin(), chains.end());
  EXPECT_EQ(1, it->function_id());

  // Now create a Copy
  TimerInfosIterator it_copy1 = it;
  EXPECT_EQ(1, it_copy1->function_id());

  // Increase the original and check that the copy does not modify
  ++it;
  EXPECT_EQ(1, it_copy1->function_id());

  // Create a copy using the copy-constructor
  TimerInfosIterator it_copy2(it_copy1);
  EXPECT_EQ(1, it_copy2->function_id());

  // Increase the original and check that the copy does not modify
  ++it_copy1;
  EXPECT_EQ(1, it_copy2->function_id());
}

TEST(TimerInfosIterator, Move) {
  std::vector<std::shared_ptr<orbit_client_data::TimerChain>> chains;
  std::shared_ptr<orbit_client_data::TimerChain> chain =
      std::make_shared<orbit_client_data::TimerChain>();
  TimerInfo timer;
  timer.set_function_id(1);
  chain->emplace_back(timer);
  chains.emplace_back(chain);

  // Now create an iterator and test to access it
  TimerInfosIterator it(chains.begin(), chains.end());
  EXPECT_EQ(1, it->function_id());

  // Now create a Copy
  TimerInfosIterator it_copy1 = it;
  EXPECT_EQ(1, it_copy1->function_id());

  // Create a copy using the copy-constructor
  TimerInfosIterator it_copy2(it_copy1);
  EXPECT_EQ(1, it_copy2->function_id());
}

TEST(TimerInfosIterator, Equality) {
  std::vector<std::shared_ptr<orbit_client_data::TimerChain>> chains;
  std::shared_ptr<orbit_client_data::TimerChain> chain =
      std::make_shared<orbit_client_data::TimerChain>();
  TimerInfo timer;
  timer.set_function_id(1);
  chain->emplace_back(timer);
  chains.emplace_back(chain);

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
  std::vector<std::shared_ptr<orbit_client_data::TimerChain>> chains;

  TimerInfosIterator it_begin(chains.begin(), chains.end());
  TimerInfosIterator it_end(chains.begin(), chains.end());

  std::vector<uint64_t> result;
  for (auto it = it_begin; it != it_end; ++it) {
    result.emplace_back(it->function_id());
  }
  EXPECT_THAT(result, testing::ElementsAre());
}

TEST(TimerInfosIterator, ForEachLarge) {
  std::vector<std::shared_ptr<orbit_client_data::TimerChain>> chains;
  std::vector<uint64_t> expected;

  size_t count = 0;
  size_t max_timers = 32;
  // Add twelve chains, with the size 2⁴, 2⁵, 2⁶, ...
  //  such that there are blocks inside the chains that are full (1024 elements),
  //  as well as blocks that are not full.
  for (size_t chain_count = 0; chain_count < 12; ++chain_count) {
    std::shared_ptr<orbit_client_data::TimerChain> chain =
        std::make_shared<orbit_client_data::TimerChain>();
    for (size_t timer_count = 0; timer_count < max_timers; ++timer_count) {
      TimerInfo timer;
      timer.set_function_id(count);
      timer.set_start(count);
      timer.set_end(count + 1);
      chain->emplace_back(timer);
      expected.emplace_back(count);
      ++count;
    }
    max_timers *= 2;
    chains.emplace_back(chain);
  }

  TimerInfosIterator it_begin(chains.begin(), chains.end());
  TimerInfosIterator it_end(chains.end(), chains.end());

  std::vector<uint64_t> result;
  for (auto it = it_begin; it != it_end; ++it) {
    result.emplace_back(it->function_id());
  }
  EXPECT_THAT(result, testing::ElementsAreArray(expected));
}
