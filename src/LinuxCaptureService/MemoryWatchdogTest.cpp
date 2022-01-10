// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/time/time.h>
#include <gtest/gtest.h>

#include <optional>

#include "MemoryWatchdog.h"

namespace orbit_linux_capture_service {

TEST(MemoryWatchdog, GetPhysicalMemoryInBytesReturnsReasonableValues) {
  uint64_t mem_total = GetPhysicalMemoryInBytes();
  EXPECT_GE(mem_total, 1024ULL * 1024 * 1024);
}

TEST(MemoryWatchdog, ExtractRssInPagesFromProcPidStatReturnsValueWhenRssIsWellFormed) {
  static constexpr std::string_view kProcPidStat{
      "2495075 (LinuxCaptureSer) S 321797 2495075 321797 34823 2495075 1077936128 208 0 0 0 0 0 0 "
      "0 20 0 2 0 185687468 82644992 454 18446744073709551615 93904073928704 93904074590349 "
      "140722755556992 0 0 0 0 0 0 0 0 0 17 46 0 0 0 0 0 93904074765696 93904074778896 "
      "93904095248384 140722755562685 140722755562793 140722755562793 140722755567550 0"};
  std::optional<uint64_t> rss_pages = ExtractRssInPagesFromProcPidStat(kProcPidStat);
  ASSERT_TRUE(rss_pages.has_value());
  EXPECT_EQ(rss_pages, 454);
}

TEST(MemoryWatchdog, ExtractRssInPagesFromProcPidStatReturnsNulloptWhenRssIsMalformed) {
  static constexpr std::string_view kProcPidStat{
      "2495075 (LinuxCaptureSer) S 321797 2495075 321797 34823 2495075 1077936128 208 0 0 0 0 0 0 "
      "0 20 0 2 0 185687468 82644992 abc 18446744073709551615 93904073928704 93904074590349 "
      "140722755556992 0 0 0 0 0 0 0 0 0 17 46 0 0 0 0 0 93904074765696 93904074778896 "
      "93904095248384 140722755562685 140722755562793 140722755562793 140722755567550 0"};
  EXPECT_FALSE(ExtractRssInPagesFromProcPidStat(kProcPidStat).has_value());
}

TEST(MemoryWatchdog, ExtractRssInPagesFromProcPidStatReturnsNulloptWhenRssIsNotPresent) {
  static constexpr std::string_view kProcPidStat{
      "2495075 (LinuxCaptureSer) S 321797 2495075 321797 34823 2495075 1077936128 208 0 0 0 0 0 0 "
      "0 20 0 2 0 185687468 82644992"};
  EXPECT_FALSE(ExtractRssInPagesFromProcPidStat(kProcPidStat).has_value());
}

static void IncreaseRss(uint64_t amount_bytes) {
  // The memory leak is intended: if the test is run again in the same process (e.g., with
  // --gtest_repeat) we want new memory to be allocated, not the previous one to be reused.
  void* ptr = malloc(amount_bytes);  // NOLINT
  auto* typed_ptr = static_cast<volatile uint64_t*>(ptr);
  for (uint64_t i = 0; i < amount_bytes / sizeof(uint64_t); ++i) {
    typed_ptr[i] = i;
  }
}

TEST(MemoryWatchdog, ReadRssInBytesFromProcPidStatReturnsIncreasingValuesOnRssIncrease) {
  std::optional<uint64_t> rss = ReadRssInBytesFromProcPidStat();
  ASSERT_TRUE(rss.has_value());
  EXPECT_GT(rss.value(), 0);

  static constexpr uint64_t kRssIncrease = 8ULL * 1024 * 1024;
  static constexpr uint64_t kRssIncreaseTolerance = kRssIncrease / 8;
  IncreaseRss(kRssIncrease);
  std::optional<uint64_t> new_rss = ReadRssInBytesFromProcPidStat();
  ASSERT_TRUE(new_rss.has_value());
  EXPECT_GT(new_rss.value(), 0);
  EXPECT_GE(new_rss.value(), rss.value() + kRssIncrease - kRssIncreaseTolerance);
}

}  // namespace orbit_linux_capture_service
