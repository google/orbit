// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/substitute.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "GrpcProtos/Constants.h"
#include "MemoryTracingUtils.h"

using orbit_grpc_protos::kMissingInfo;
using orbit_grpc_protos::SystemMemoryUsage;

namespace {

void ExpectMemInfoParsingResult(std::optional<SystemMemoryUsage> parsing_result,
                                bool parsing_succeed, int expected_total = kMissingInfo,
                                int expected_free = kMissingInfo,
                                int expected_available = kMissingInfo,
                                int expected_buffers = kMissingInfo,
                                int expected_cached = kMissingInfo) {
  if (!parsing_succeed) {
    EXPECT_FALSE(parsing_result.has_value());
    return;
  }

  EXPECT_TRUE(parsing_result.has_value());
  EXPECT_EQ(parsing_result.value().total_kb(), expected_total);
  EXPECT_EQ(parsing_result.value().free_kb(), expected_free);
  EXPECT_EQ(parsing_result.value().available_kb(), expected_available);
  EXPECT_EQ(parsing_result.value().buffers_kb(), expected_buffers);
  EXPECT_EQ(parsing_result.value().cached_kb(), expected_cached);
}

}  // namespace

namespace orbit_memory_tracing {

TEST(MemoryUtils, ParseMemInfo) {
  const int kMemTotalKB = 16396576;
  const int kMemFreeKB = 11493816;
  const int kMemAvailableKB = 14378752;
  const int kBuffersKB = 71540;
  const int kCachedKB = 3042860;
  const int kMemTotalKiB = 16012281;
  const int kMemFreeMB = 11494;

  const float kKiloBytes = 1000;
  const float kKibiBytes = 1024;
  const float kMegaBytes = 1000000;

  const std::string kValidMeminfo =
      absl::Substitute(R"(MemTotal:       $0 kB
MemFree:        $1 kB
MemAvailable:   $2 kB
Buffers:        $3 kB
Cached:         $4 kB
SwapCached:            0 kB
Active:          3350508 kB
Inactive:        1190988 kB
Active(anon):    1444908 kB
Inactive(anon):      516 kB
Active(file):    1905600 kB
Inactive(file):  1190472 kB
Unevictable:       56432 kB
Mlocked:           56432 kB
SwapTotal:       1953788 kB
SwapFree:        1953788 kB
Dirty:               492 kB
Writeback:             0 kB
AnonPages:       1326896 kB
Mapped:           716656 kB
Shmem:               796 kB
KReclaimable:      84864 kB
Slab:             194376 kB
SReclaimable:      84864 kB
SUnreclaim:       109512 kB
KernelStack:       24724 kB
PageTables:        13164 kB
NFS_Unstable:          0 kB
Bounce:                0 kB
WritebackTmp:          0 kB
CommitLimit:    10152076 kB
Committed_AS:    6324736 kB
VmallocTotal:   34359738367 kB
VmallocUsed:       38264 kB
VmallocChunk:          0 kB
Percpu:             3072 kB
HardwareCorrupted:     0 kB
AnonHugePages:    782336 kB
ShmemHugePages:        0 kB
ShmemPmdMapped:        0 kB
FileHugePages:         0 kB
FilePmdMapped:         0 kB
HugePages_Total:       0
HugePages_Free:        0
HugePages_Rsvd:        0
HugePages_Surp:        0
Hugepagesize:       2048 kB
Hugetlb:               0 kB
DirectMap4k:      201960 kB
DirectMap2M:     5040128 kB
DirectMap1G:    13631488 kB)",
                       kMemTotalKB, kMemFreeKB, kMemAvailableKB, kBuffersKB, kCachedKB);

  const std::string kPartialMeminfo = absl::Substitute(R"(MemTotal:       $0 kB
MemFree:        $1 kB
SwapCached:      0 kB)",
                                                       kMemTotalKB, kMemFreeKB);

  const std::string kEmptyMeminfo = "";

  const std::string kPartialMeminfoWithDifferentSizeUnits =
      absl::Substitute(R"(MemTotal:       $0 KiB
MemFree:        $1 MB
SwapCached:      0 kB)",
                       kMemTotalKiB, kMemFreeMB);

  std::optional<SystemMemoryUsage> parsing_result;

  parsing_result = ParseMemInfo(kValidMeminfo);
  ExpectMemInfoParsingResult(parsing_result, true, kMemTotalKB, kMemFreeKB, kMemAvailableKB,
                             kBuffersKB, kCachedKB);

  parsing_result = ParseMemInfo(kPartialMeminfo);
  ExpectMemInfoParsingResult(parsing_result, true, kMemTotalKB, kMemFreeKB);

  parsing_result = ParseMemInfo(kEmptyMeminfo);
  ExpectMemInfoParsingResult(parsing_result, false);

  parsing_result = ParseMemInfo(kPartialMeminfoWithDifferentSizeUnits);
  int converted_mem_total_kb = static_cast<int>(std::round(kKibiBytes / kKiloBytes * kMemTotalKiB));
  int converted_mem_free_kb = static_cast<int>(std::round(kMegaBytes / kKiloBytes * kMemFreeMB));
  ExpectMemInfoParsingResult(parsing_result, true, converted_mem_total_kb, converted_mem_free_kb);
}

}  // namespace orbit_memory_tracing