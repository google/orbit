// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <sys/types.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <numeric>
#include <optional>

#include "Histogram.h"

namespace orbit_statistics {

class HistogramBuilderTest : public ::testing::Test {
 public:
  HistogramBuilder& GetObjectUnderTest() { return histogram_builder; }

  std::optional<uint64_t> GetMin() { return GetObjectUnderTest().min_; }

  std::optional<uint64_t> GetMax() { return GetObjectUnderTest().max_; }

  void SetMin(uint64_t min) { GetObjectUnderTest().min_ = min; }

  void SetMax(uint64_t max) { GetObjectUnderTest().max_ = max; }

  std::optional<uint64_t> GetBandwidth() { return GetObjectUnderTest().bandwidth_; }

  size_t ValueToIndex(uint64_t value) { return GetObjectUnderTest().ValueToIndex(value); }

  const std::vector<uint64_t>* GetDataSet() { return GetObjectUnderTest().data_; }

 protected:
  void SetUp() override {}

 private:
  HistogramBuilder histogram_builder;
};

constexpr uint64_t kBandwidth = 5;
constexpr size_t kDataSetSize = 8;
constexpr std::array<uint64_t, kDataSetSize> kDataSet{11ULL, 12ULL, 14ULL,  18ULL,
                                                      19ULL, 30ULL, 100ULL, 58ULL};
constexpr uint64_t kMin = *std::min_element(kDataSet.begin(), kDataSet.end());
constexpr uint64_t kMax = *std::max_element(kDataSet.begin(), kDataSet.end());

const std::vector<uint64_t> data_set(kDataSet.begin(), kDataSet.end());

TEST_F(HistogramBuilderTest, InitializationTest) {
  EXPECT_EQ(GetMin(), std::nullopt);
  EXPECT_EQ(GetMax(), std::nullopt);
  EXPECT_EQ(GetBandwidth(), std::nullopt);
  EXPECT_EQ(GetDataSet(), nullptr);
}

TEST_F(HistogramBuilderTest, SetBandwidthTest) {
  GetObjectUnderTest().SetBandwidth(kBandwidth);

  EXPECT_EQ(GetBandwidth(), kBandwidth);
}

TEST_F(HistogramBuilderTest, SetDataSetTest) {
  GetObjectUnderTest().SetDataSet(&data_set);

  EXPECT_EQ(GetMin(), kMin);
  EXPECT_EQ(GetMax(), kMax);
  EXPECT_EQ(GetDataSet(), &data_set);
}

TEST_F(HistogramBuilderTest, ValueToIndexTest) {
  GetObjectUnderTest().SetDataSet(&data_set);
  GetObjectUnderTest().SetBandwidth(kBandwidth);

  EXPECT_EQ(ValueToIndex(15ULL), 0);
  EXPECT_EQ(ValueToIndex(14ULL), 0);
  EXPECT_EQ(ValueToIndex(16ULL), 1);
  EXPECT_EQ(ValueToIndex(25ULL), 2);
  EXPECT_EQ(ValueToIndex(24ULL), 2);
  EXPECT_EQ(ValueToIndex(26ULL), 3);
  EXPECT_EQ(ValueToIndex(100ULL), 17);
}

TEST_F(HistogramBuilderTest, TestCounting) {
  GetObjectUnderTest().SetDataSet(&data_set);
  GetObjectUnderTest().SetBandwidth(kBandwidth);

  auto histogram = GetObjectUnderTest().Build();
  EXPECT_EQ(histogram->data_set_size, kDataSetSize);
  EXPECT_EQ(histogram->min, kMin);
  EXPECT_EQ(histogram->max, kMax);
  EXPECT_EQ(histogram->bandwidth, kBandwidth);

  const std::vector<size_t>& counts = histogram->counts;
  EXPECT_EQ(counts.size(), 18);
  EXPECT_EQ(counts[0], 3);
  EXPECT_EQ(counts[1], 2);
  EXPECT_EQ(counts[3], 1);
  EXPECT_EQ(counts[9], 1);
  EXPECT_EQ(counts[17], 1);

  EXPECT_EQ(std::reduce(counts.begin(), counts.end()), kDataSetSize);
}

TEST_F(HistogramBuilderTest, TestCountingAllEqual) {
  const size_t singular_dataset_size = 100;
  const std::vector<uint64_t> singular_data_set(singular_dataset_size, 5ULL);
  GetObjectUnderTest().SetDataSet(&singular_data_set);
  GetObjectUnderTest().SetBandwidth(kBandwidth);

  auto histogram = GetObjectUnderTest().Build();
  EXPECT_EQ(histogram->data_set_size, singular_dataset_size);
  EXPECT_EQ(histogram->min, 5);
  EXPECT_EQ(histogram->max, 5);

  EXPECT_EQ(histogram->counts.size(), 1);
  EXPECT_EQ(histogram->counts[0], singular_dataset_size);
}

TEST_F(HistogramBuilderTest, NumberOfBinsCorrectlySetsBandidthWithOverflow) {
  SetMin(1ULL);
  SetMax(7ULL);
  GetObjectUnderTest().SetNumberOfBins(2);

  EXPECT_EQ(GetBandwidth(), 4);
}

TEST_F(HistogramBuilderTest, NumberOfBinsCorrectlySetsBandidthWithoutOverflow) {
  SetMin(1ULL);
  SetMax(6ULL);
  GetObjectUnderTest().SetNumberOfBins(2);

  EXPECT_EQ(GetBandwidth(), 3);
}

TEST_F(HistogramBuilderTest, NumberOfBinsCorrectlySetsBandidthForExcessiveNumberOfBins) {
  SetMin(1ULL);
  SetMax(6ULL);
  GetObjectUnderTest().SetNumberOfBins(200);

  EXPECT_EQ(GetBandwidth(), 6);
}

TEST_F(HistogramBuilderTest, NumberOfBinsCorrectlySetsBandidthWhenWidthEqualsNumberOfBins) {
  SetMin(1ULL);
  SetMax(6ULL);
  GetObjectUnderTest().SetNumberOfBins(6);

  EXPECT_EQ(GetBandwidth(), 1);
}

TEST_F(HistogramBuilderTest, NumberOfBinsCorrectlySetsBandidthWhenNumberOfBinsBinsEqualsOne) {
  SetMin(1ULL);
  SetMax(6ULL);
  GetObjectUnderTest().SetNumberOfBins(1);

  EXPECT_EQ(GetBandwidth(), 6);
}

}  // namespace orbit_statistics