// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <numeric>
#include <vector>

#include "HistogramPrivate.h"
#include "Statistics/DataSet.h"
#include "Statistics/Histogram.h"

namespace orbit_statistics {

constexpr uint64_t kBandwidth = 5;
constexpr size_t kDataSetSize = 8;
constexpr std::array<uint64_t, kDataSetSize> kRawDataSet{11ULL, 12ULL, 14ULL,  18ULL,
                                                         19ULL, 30ULL, 100ULL, 58ULL};
constexpr uint64_t kMin = *std::min_element(kRawDataSet.begin(), kRawDataSet.end());
constexpr uint64_t kMax = *std::max_element(kRawDataSet.begin(), kRawDataSet.end());

const std::vector<uint64_t> raw_data_set(kRawDataSet.begin(), kRawDataSet.end());

TEST(CrteateDataSetTest, TestWithEmptyVector) {
  const std::vector<uint64_t> empty;
  const std::optional<DataSet> data_set = CreateDataSet(&empty);
  EXPECT_TRUE(!data_set.has_value());
}

TEST(CrteateDataSetTest, TestWithNonEmptyVector) {
  const std::optional<DataSet> data_set = CreateDataSet(&raw_data_set);
  EXPECT_TRUE(data_set.has_value());
  EXPECT_EQ(data_set->GetMin(), kMin);
  EXPECT_EQ(data_set->GetMax(), kMax);
}

TEST(HistogramTest, ValueToIndexTest) {
  const std::optional<DataSet> data_set = CreateDataSet(&raw_data_set);

  EXPECT_EQ(ValueToIndex(15ULL, data_set.value(), kBandwidth), 0);
  EXPECT_EQ(ValueToIndex(14ULL, data_set.value(), kBandwidth), 0);
  EXPECT_EQ(ValueToIndex(16ULL, data_set.value(), kBandwidth), 1);
  EXPECT_EQ(ValueToIndex(25ULL, data_set.value(), kBandwidth), 2);
  EXPECT_EQ(ValueToIndex(24ULL, data_set.value(), kBandwidth), 2);
  EXPECT_EQ(ValueToIndex(26ULL, data_set.value(), kBandwidth), 3);
  EXPECT_EQ(ValueToIndex(100ULL, data_set.value(), kBandwidth), 17);
}

TEST(HistogramBuildTest, TestCounting) {
  const std::optional<DataSet> data_set = CreateDataSet(&raw_data_set);

  const auto histogram = BuildHistogram(data_set.value(), kBandwidth);
  EXPECT_EQ(histogram.data_set_size, kDataSetSize);
  EXPECT_EQ(histogram.min, kMin);
  EXPECT_EQ(histogram.max, kMax);
  EXPECT_EQ(histogram.bandwidth, kBandwidth);

  const std::vector<size_t>& counts = histogram.counts;
  EXPECT_EQ(counts.size(), 18);
  EXPECT_EQ(counts[0], 3);
  EXPECT_EQ(counts[1], 2);
  EXPECT_EQ(counts[3], 1);
  EXPECT_EQ(counts[9], 1);
  EXPECT_EQ(counts[17], 1);

  EXPECT_EQ(std::reduce(counts.begin(), counts.end()), kDataSetSize);
}

TEST(HistogramBuildTest, TestCountingAllEqual) {
  const size_t singular_dataset_size = 100;
  const std::vector<uint64_t> singular_raw_data_set(singular_dataset_size, 5ULL);
  const auto data_set = CreateDataSet(&singular_raw_data_set);

  auto histogram = BuildHistogram(data_set.value(), kBandwidth);
  EXPECT_EQ(histogram.data_set_size, singular_dataset_size);
  EXPECT_EQ(histogram.min, 5);
  EXPECT_EQ(histogram.max, 5);

  EXPECT_EQ(histogram.counts.size(), 1);
  EXPECT_EQ(histogram.counts[0], singular_dataset_size);
}

uint64_t NumberOfBinsToBandwidth(size_t bins_num, uint64_t max, uint64_t min) {
  const std::vector<uint64_t> raw_data = {max, min};
  const auto data_set = CreateDataSet(&raw_data);
  return NumberOfBinsToBandwidth(data_set.value(), bins_num);
}

TEST(NumberOfBinsToBandwidthTest, NumberOfBinsCorrectlySetsBandidthWithOverflow) {
  EXPECT_EQ(NumberOfBinsToBandwidth(2, 1ULL, 7ULL), 4);
}

TEST(NumberOfBinsToBandwidthTest, NumberOfBinsCorrectlySetsBandidthWithoutOverflow) {
  EXPECT_EQ(NumberOfBinsToBandwidth(2, 1ULL, 6ULL), 3);
}

TEST(NumberOfBinsToBandwidthTest, NumberOfBinsCorrectlySetsBandidthForExcessiveNumberOfBins) {
  EXPECT_EQ(NumberOfBinsToBandwidth(200, 1ULL, 6ULL), 6);
}

TEST(NumberOfBinsToBandwidthTest, NumberOfBinsCorrectlySetsBandidthWhenWidthEqualsNumberOfBins) {
  EXPECT_EQ(NumberOfBinsToBandwidth(6, 1ULL, 6ULL), 1);
}

TEST(NumberOfBinsToBandwidthTest, NumberOfBinsCorrectlySetsBandidthWhenNumberOfBinsBinsEqualsOne) {
  EXPECT_EQ(NumberOfBinsToBandwidth(1, 1ULL, 6ULL), 6);
}

TEST(HistogramTest, RiskScoreTest) {
  const uint64_t bandwidth = 7421300;
  const std::vector<uint64_t> counts = {32ULL, 30ULL, 174ULL, 42ULL, 2ULL};
  const uint64_t min = 14015002;
  const uint64_t max = 43843646;
  const size_t data_set_size = 280;
  const Histogram histogram{min, max, bandwidth, data_set_size, counts};
  EXPECT_NEAR(HistogramRiskScore(histogram), -1.72, 0.01);
}

TEST(HistogramTest, RiskScoreTestZeroWidth) {
  const Histogram histogram{0, 0, 1, 1, {}};
  EXPECT_DOUBLE_EQ(HistogramRiskScore(histogram), 0.0);
}

TEST(BuildHistogramTest, BuildHistogramCorrectlyChoosesTheBandwidth) {
  const std::vector<uint64_t> data = {
      14015002, 14085204, 14137416, 14148620, 14208677, 14210556, 14230187, 14236563, 14249140,
      14297935, 14370241, 14483703, 14650528, 14694684, 14716937, 14731743, 14749105, 14752345,
      14790428, 15021057, 20141721, 20168268, 20186247, 20193731, 20214833, 20223157, 20223742,
      20227335, 20278999, 20292721, 20301237, 20301991, 24055404, 24057134, 24116357, 24116504,
      24131568, 24151217, 24176362, 24241821, 24714395, 24767102, 24770594, 24805187, 24822433,
      24850160, 24869143, 24911895, 24924603, 24949718, 24986096, 25015055, 25040115, 25068103,
      25114907, 25130083, 28635207, 28691905, 28750123, 28752509, 28834211, 28853815, 28888116,
      28890270, 28906419, 28920365, 28933352, 28940015, 28972590, 29029981, 29031401, 29270889,
      29358674, 29455073, 29502428, 29504583, 29509132, 29548314, 29555329, 29567029, 29572911,
      29582240, 29590375, 29592986, 29595526, 29602560, 29634093, 29668103, 29668619, 29696274,
      29700181, 29704368, 29711153, 29834221, 29849448, 29888558, 29899288, 29966501, 30002077,
      30028100, 30109007, 30135920, 30177700, 30187167, 30188577, 30193668, 30194939, 30228287,
      30236728, 30267653, 30268008, 30272628, 30273564, 30275091, 30284565, 30295982, 30298296,
      30306734, 30310385, 30312385, 30328374, 30329804, 30335749, 30336779, 30336972, 30338644,
      30363882, 30365100, 30377248, 30396709, 30435982, 30445014, 30502529, 30515053, 30582631,
      30589360, 33345631, 33405886, 33432758, 33444982, 33555403, 33584631, 33594606, 33645082,
      33685798, 33761537, 33780738, 33832544, 33841409, 33899295, 33918235, 33959054, 33965251,
      34005959, 34017296, 34044961, 34064117, 34067247, 34079237, 34090188, 34090827, 34092588,
      34114904, 34124307, 34176691, 34179685, 34189613, 34214479, 34214660, 34222729, 34223168,
      34223771, 34233317, 34247607, 34257133, 34269842, 34291111, 34300764, 34302060, 34305574,
      34323931, 34324748, 34326404, 34355459, 34357010, 34360424, 34371425, 34377468, 34420619,
      34445544, 34460879, 34462581, 34473560, 34479394, 34489778, 34502197, 34504637, 34548359,
      34567674, 34586139, 34587252, 34588714, 34589465, 34615493, 34627455, 34631143, 34633700,
      34639764, 34648712, 34653522, 34663464, 34686831, 34698437, 34706436, 34715928, 34715952,
      34733597, 34749597, 34749843, 34776083, 34801938, 34823808, 34837046, 34908830, 34924444,
      34989097, 34994754, 35132292, 35166497, 35221829, 35250329, 35282934, 35288797, 35362324,
      35616563, 35758245, 38460533, 38463941, 38524820, 38572781, 38968265, 39039440, 39054834,
      39089852, 39123814, 39144487, 39163219, 39183044, 39222517, 39234791, 39246342, 39249978,
      39260702, 39270784, 39300885, 39315154, 39321755, 39335340, 39337847, 39343184, 39343354,
      39400519, 39408143, 39483506, 39521073, 39557353, 39577570, 39577586, 39599664, 39627373,
      39884349, 39889010, 39925959, 39933440, 39987806, 40223294, 43078564, 43081818, 43700203,
      43843646};

  std::optional<Histogram> hist = BuildHistogram(data);
  EXPECT_EQ(hist->counts.size(), 128);
}

}  // namespace orbit_statistics