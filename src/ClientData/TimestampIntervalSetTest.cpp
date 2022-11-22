// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <map>
#include <vector>

#include "ClientData/TimestampIntervalSet.h"

using ::testing::ElementsAre;

namespace orbit_client_data {

MATCHER_P2(TimestampIntervalEq, start_inclusive, end_exclusive, "") {
  const TimestampIntervalSet::TimestampInterval& interval = arg;
  return interval.start_inclusive() == static_cast<uint64_t>(start_inclusive) &&
         interval.end_exclusive() == static_cast<uint64_t>(end_exclusive);
}

TEST(TimestampIntervalSet, EmptyAndSize) {
  TimestampIntervalSet set;
  EXPECT_TRUE(set.empty());
  EXPECT_EQ(set.size(), 0);

  set.Add(5, 10);
  EXPECT_FALSE(set.empty());
  EXPECT_EQ(set.size(), 1);

  set.Add(15, 20);
  EXPECT_FALSE(set.empty());
  EXPECT_EQ(set.size(), 2);
}

TEST(TimestampIntervalSet, Add) {
  TimestampIntervalSet set;
  EXPECT_DEATH(set.Add(5, 4), "");
  EXPECT_DEATH(set.Add(5, 5), "");

  set.Add(5, 10);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 10)));
  set.Add(15, 20);
  EXPECT_THAT(std::vector(set.begin(), set.end()),
              ElementsAre(TimestampIntervalEq(5, 10), TimestampIntervalEq(15, 20)));

  set.Clear();
  set.Add(5, 10);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 10)));
  set.Add(11, 20);
  EXPECT_THAT(std::vector(set.begin(), set.end()),
              ElementsAre(TimestampIntervalEq(5, 10), TimestampIntervalEq(11, 20)));

  set.Clear();
  set.Add(11, 20);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(11, 20)));
  set.Add(5, 10);
  EXPECT_THAT(std::vector(set.begin(), set.end()),
              ElementsAre(TimestampIntervalEq(5, 10), TimestampIntervalEq(11, 20)));

  set.Clear();
  set.Add(5, 10);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 10)));
  set.Add(10, 20);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 20)));
  set.Clear();

  set.Clear();
  set.Add(10, 20);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(10, 20)));
  set.Add(5, 10);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 20)));

  set.Clear();
  set.Add(5, 10);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 10)));
  set.Add(0, 20);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(0, 20)));

  set.Clear();
  set.Add(0, 20);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(0, 20)));
  set.Add(5, 10);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(0, 20)));

  set.Clear();
  set.Add(5, 10);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 10)));
  set.Add(15, 20);
  EXPECT_THAT(std::vector(set.begin(), set.end()),
              ElementsAre(TimestampIntervalEq(5, 10), TimestampIntervalEq(15, 20)));
  set.Add(10, 15);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 20)));

  set.Clear();
  set.Add(5, 10);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 10)));
  set.Add(15, 20);
  EXPECT_THAT(std::vector(set.begin(), set.end()),
              ElementsAre(TimestampIntervalEq(5, 10), TimestampIntervalEq(15, 20)));
  set.Add(9, 16);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 20)));

  set.Clear();
  set.Add(5, 10);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 10)));
  set.Add(15, 20);
  EXPECT_THAT(std::vector(set.begin(), set.end()),
              ElementsAre(TimestampIntervalEq(5, 10), TimestampIntervalEq(15, 20)));
  set.Add(5, 20);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 20)));

  set.Clear();
  set.Add(5, 10);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 10)));
  set.Add(15, 20);
  EXPECT_THAT(std::vector(set.begin(), set.end()),
              ElementsAre(TimestampIntervalEq(5, 10), TimestampIntervalEq(15, 20)));
  set.Add(25, 30);
  EXPECT_THAT(std::vector(set.begin(), set.end()),
              ElementsAre(TimestampIntervalEq(5, 10), TimestampIntervalEq(15, 20),
                          TimestampIntervalEq(25, 30)));
  set.Add(9, 26);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 30)));

  set.Clear();
  set.Add(5, 10);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 10)));
  set.Add(5, 10);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 10)));
  set.Add(5, 9);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 10)));
  set.Add(5, 11);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(5, 11)));
  set.Add(4, 11);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(4, 11)));
  set.Add(3, 5);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(3, 11)));
  set.Add(15, 20);
  EXPECT_THAT(std::vector(set.begin(), set.end()),
              ElementsAre(TimestampIntervalEq(3, 11), TimestampIntervalEq(15, 20)));
  set.Add(14, 20);
  EXPECT_THAT(std::vector(set.begin(), set.end()),
              ElementsAre(TimestampIntervalEq(3, 11), TimestampIntervalEq(14, 20)));
  set.Add(14, 21);
  EXPECT_THAT(std::vector(set.begin(), set.end()),
              ElementsAre(TimestampIntervalEq(3, 11), TimestampIntervalEq(14, 21)));
  set.Add(4, 12);
  EXPECT_THAT(std::vector(set.begin(), set.end()),
              ElementsAre(TimestampIntervalEq(3, 12), TimestampIntervalEq(14, 21)));
  set.Add(3, 13);
  EXPECT_THAT(std::vector(set.begin(), set.end()),
              ElementsAre(TimestampIntervalEq(3, 13), TimestampIntervalEq(14, 21)));
  set.Add(3, 14);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(3, 21)));
  set.Add(25, 30);
  EXPECT_THAT(std::vector(set.begin(), set.end()),
              ElementsAre(TimestampIntervalEq(3, 21), TimestampIntervalEq(25, 30)));
  set.Add(35, 40);
  EXPECT_THAT(std::vector(set.begin(), set.end()),
              ElementsAre(TimestampIntervalEq(3, 21), TimestampIntervalEq(25, 30),
                          TimestampIntervalEq(35, 40)));
  set.Add(17, 37);
  EXPECT_THAT(std::vector(set.begin(), set.end()), ElementsAre(TimestampIntervalEq(3, 40)));
}

TEST(TimestampIntervalSet, LowerBound) {
  TimestampIntervalSet set;
  EXPECT_EQ(set.LowerBound(10), set.begin());
  EXPECT_EQ(set.LowerBound(10), set.end());

  set.Add(5, 10);  // Current set: [5, 10)
  EXPECT_EQ(set.LowerBound(0), set.begin());
  EXPECT_THAT(*set.LowerBound(0), TimestampIntervalEq(5, 10));
  EXPECT_EQ(set.LowerBound(5), set.begin());
  EXPECT_THAT(*set.LowerBound(5), TimestampIntervalEq(5, 10));
  EXPECT_EQ(set.LowerBound(9), set.begin());
  EXPECT_THAT(*set.LowerBound(9), TimestampIntervalEq(5, 10));
  EXPECT_EQ(set.LowerBound(10), set.end());
  EXPECT_EQ(set.LowerBound(15), set.end());

  set.Add(15, 20);  // Current set: [5, 10), [15, 20)
  EXPECT_EQ(set.LowerBound(0), set.begin());
  EXPECT_THAT(*set.LowerBound(0), TimestampIntervalEq(5, 10));
  EXPECT_EQ(set.LowerBound(5), set.begin());
  EXPECT_THAT(*set.LowerBound(5), TimestampIntervalEq(5, 10));
  EXPECT_EQ(set.LowerBound(9), set.begin());
  EXPECT_THAT(*set.LowerBound(9), TimestampIntervalEq(5, 10));
  EXPECT_THAT(*set.LowerBound(10), TimestampIntervalEq(15, 20));
  EXPECT_THAT(*set.LowerBound(14), TimestampIntervalEq(15, 20));
  EXPECT_THAT(*set.LowerBound(15), TimestampIntervalEq(15, 20));
  EXPECT_THAT(*set.LowerBound(19), TimestampIntervalEq(15, 20));
  EXPECT_EQ(set.LowerBound(20), set.end());
  EXPECT_EQ(set.LowerBound(25), set.end());
}

}  // namespace orbit_client_data
