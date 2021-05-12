// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/substitute.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "GrpcProtos/Constants.h"
#include "MemoryTracingUtils.h"

using orbit_grpc_protos::CGroupMemoryUsage;
using orbit_grpc_protos::kMissingInfo;
using orbit_grpc_protos::SystemMemoryUsage;

namespace {

void ExpectSystemMemoryUsageEq(const SystemMemoryUsage& system_memory_usage,
                               int expected_total = kMissingInfo, int expected_free = kMissingInfo,
                               int expected_available = kMissingInfo,
                               int expected_buffers = kMissingInfo,
                               int expected_cached = kMissingInfo) {
  EXPECT_EQ(system_memory_usage.total_kb(), expected_total);
  EXPECT_EQ(system_memory_usage.free_kb(), expected_free);
  EXPECT_EQ(system_memory_usage.available_kb(), expected_available);
  EXPECT_EQ(system_memory_usage.buffers_kb(), expected_buffers);
  EXPECT_EQ(system_memory_usage.cached_kb(), expected_cached);
}

void ResetCGroupMemoryUsage(CGroupMemoryUsage* cgroup_memory_usage) {
  cgroup_memory_usage->set_limit_bytes(kMissingInfo);
  cgroup_memory_usage->set_rss_bytes(kMissingInfo);
  cgroup_memory_usage->set_mapped_file_bytes(kMissingInfo);
}

void ExpectCGroupMemoryUsageEq(const CGroupMemoryUsage& cgroup_memory_usage,
                               int limit_bytes = kMissingInfo, int rss_bytes = kMissingInfo,
                               int mapped_file_bytes = kMissingInfo) {
  EXPECT_EQ(cgroup_memory_usage.limit_bytes(), limit_bytes);
  EXPECT_EQ(cgroup_memory_usage.rss_bytes(), rss_bytes);
  EXPECT_EQ(cgroup_memory_usage.mapped_file_bytes(), mapped_file_bytes);
}

}  // namespace

namespace orbit_memory_tracing {

TEST(MemoryUtils, ParseMemInfo) {
  const int kMemTotal = 16396576;
  const int kMemFree = 11493816;
  const int kMemAvailable = 14378752;
  const int kBuffers = 71540;
  const int kCached = 3042860;

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
                       kMemTotal, kMemFree, kMemAvailable, kBuffers, kCached);

  const std::string kPartialMeminfo = absl::Substitute(R"(MemTotal:       $0 kB
MemFree:        $1 kB
SwapCached:      0 kB)",
                                                       kMemTotal, kMemFree);

  const std::string kEmptyMeminfo = "";

  SystemMemoryUsage parsing_result = ParseMemInfo(kValidMeminfo);
  ExpectSystemMemoryUsageEq(parsing_result, kMemTotal, kMemFree, kMemAvailable, kBuffers, kCached);

  parsing_result = ParseMemInfo(kPartialMeminfo);
  ExpectSystemMemoryUsageEq(parsing_result, kMemTotal, kMemFree);

  parsing_result = ParseMemInfo(kEmptyMeminfo);
  ExpectSystemMemoryUsageEq(parsing_result);
}

TEST(MemoryUtils, GetVmRssFromProcessStatus) {
  const int kVmRSS = 11268;

  const std::string kValidProcessStatus = absl::Substitute(R"(Name:	java
Umask:	0027
State:	S (sleeping)
Tgid:	816242
Ngid:	0
Pid:	816242
PPid:	816101
TracerPid:	0
Uid:	475321	475321	475321	475321
Gid:	89939	89939	89939	89939
FDSize:	1024
Groups:	4 20 24 25 44 46 109 129 997 5000 66688 70967 70970 74990 75209
NStgid:	816242
NSpid:	816242
NSpgid:	981
NSsid:	981
VmPeak:	 9700792 kB
VmSize:	 9654768 kB
VmLck:	      16 kB
VmPin:	       0 kB
VmHWM:	 1945972 kB
VmRSS:	 $0 kB
RssAnon:	 1598556 kB
RssFile:	   47672 kB
RssShmem:	     224 kB
VmData:	 2178668 kB
VmStk:	     136 kB
VmExe:	       4 kB
VmLib:	  155960 kB
VmPTE:	    4744 kB
VmSwap:	  248780 kB
HugetlbPages:	       0 kB
CoreDumping:	0
THP_enabled:	1
Threads:	76
SigQ:	0/63711
SigPnd:	0000000000000000
ShdPnd:	0000000000000000
SigBlk:	0000000000000000
SigIgn:	0000000000000000
SigCgt:	2000000181005ccf
CapInh:	0000000000000000
CapPrm:	0000000000000000
CapEff:	0000000000000000
CapBnd:	000001ffffffffff
CapAmb:	0000000000000000
NoNewPrivs:	0
Seccomp:	0
Seccomp_filters:	0
Speculation_Store_Bypass:	thread vulnerable
Cpus_allowed:	f
Cpus_allowed_list:	0-3
Mems_allowed:	00000000,00000000,00000000,00000001
Mems_allowed_list:	0
voluntary_ctxt_switches:	1
nonvoluntary_ctxt_switches:	3)",
                                                           kVmRSS);
  const std::string kPartialProcessStatus = R"(Name:   java
Umask:  0027)";

  const std::string kEmptyProcessStatus = "";

  int64_t parsing_result = GetVmRssFromProcessStatus(kValidProcessStatus);
  EXPECT_EQ(parsing_result, kVmRSS);

  parsing_result = GetVmRssFromProcessStatus(kPartialProcessStatus);
  EXPECT_EQ(parsing_result, kMissingInfo);

  parsing_result = GetVmRssFromProcessStatus(kEmptyProcessStatus);
  EXPECT_EQ(parsing_result, kMissingInfo);
}

TEST(MemoryUtil, GetProcessMemoryCGroupName) {
  const std::string kCGroupName = "user.slice/user-1000.slice";

  const std::string kValidProcessCGroup = absl::Substitute(R"(10:memory:/$0
9:blkio:/user.slice/user-1000.slice
8:net_cls,net_prio:/
7:cpu,cpuacct:/user.slice/user-1000.slice
6:perf_event:/
5:freezer:/
4:cpuset:/
3:pids:/user.slice/user-1000.slice
2:devices:/user.slice/user-1000.slice
1:name=systemd:/user.slice/user-1000.slice/session-3.scope)",
                                                           kCGroupName);

  const std::string kPartialProcessCGroup = R"(3:pids:/user.slice/user-1000.slice
2:devices:/user.slice/user-1000.slice
1:name=systemd:/user.slice/user-1000.slice/session-3.scope)";

  const std::string kEmptyProcessCGroup = "";

  std::string parsing_result = GetProcessMemoryCGroupName(kValidProcessCGroup);
  EXPECT_EQ(parsing_result, kCGroupName);

  parsing_result = GetProcessMemoryCGroupName(kPartialProcessCGroup);
  EXPECT_TRUE(parsing_result.empty());

  parsing_result = GetProcessMemoryCGroupName(kEmptyProcessCGroup);
  EXPECT_TRUE(parsing_result.empty());
}

TEST(MemoryUtils, GetCGroupMemoryLimitInBytes) {
  const int kCGroupMemoryLimitInBytes = 12345;

  const std::string kValidCGroupMemoryLimitInBytes = std::to_string(kCGroupMemoryLimitInBytes);
  const std::string kEmptyCGroupMemoryLimitInBytes = "";

  CGroupMemoryUsage cgroup_memory_usage;
  ResetCGroupMemoryUsage(&cgroup_memory_usage);
  GetCGroupMemoryLimitInBytes(kValidCGroupMemoryLimitInBytes, &cgroup_memory_usage);
  ExpectCGroupMemoryUsageEq(cgroup_memory_usage, kCGroupMemoryLimitInBytes);

  ResetCGroupMemoryUsage(&cgroup_memory_usage);
  GetCGroupMemoryLimitInBytes(kEmptyCGroupMemoryLimitInBytes, &cgroup_memory_usage);
  ExpectCGroupMemoryUsageEq(cgroup_memory_usage);
}

TEST(MemoryUtils, GetValuesFromCGroupMemoryStat) {
  const int kRssInBytes = 245760;
  const int KMappedFileInBytes = 1234;

  const std::string kValidCGroupMemoryStatus = absl::Substitute(R"(cache 36864
rss $0
rss_huge 0
shmem 0
mapped_file $1
dirty 135168
writeback 0
pgpgin 299
pgpgout 230
pgfault 1425
pgmajfault 0
inactive_anon 16384
active_anon 253952
inactive_file 0
active_file 12288
unevictable 0
hierarchical_memory_limit 14817636352
total_cache 36864
total_rss 245760
total_rss_huge 0
total_shmem 0
total_mapped_file 0
total_dirty 135168
total_writeback 0
total_pgpgin 299
total_pgpgout 230
total_pgfault 1425
total_pgmajfault 0
total_inactive_anon 16384
total_active_anon 253952
total_inactive_file 0
total_active_file 12288
total_unevictable 0)",
                                                                kRssInBytes, KMappedFileInBytes);

  const std::string kPartialCGroupMemoryStatus = R"(cache 36864
rss_huge 0)";

  const std::string kEmptyCGroupMemoryStatus = "";

  CGroupMemoryUsage cgroup_memory_usage;
  ResetCGroupMemoryUsage(&cgroup_memory_usage);
  GetValuesFromCGroupMemoryStat(kValidCGroupMemoryStatus, &cgroup_memory_usage);
  ExpectCGroupMemoryUsageEq(cgroup_memory_usage, kMissingInfo, kRssInBytes, KMappedFileInBytes);

  ResetCGroupMemoryUsage(&cgroup_memory_usage);
  GetValuesFromCGroupMemoryStat(kPartialCGroupMemoryStatus, &cgroup_memory_usage);
  ExpectCGroupMemoryUsageEq(cgroup_memory_usage);

  GetValuesFromCGroupMemoryStat(kEmptyCGroupMemoryStatus, &cgroup_memory_usage);
  ExpectCGroupMemoryUsageEq(cgroup_memory_usage);
}

}  // namespace orbit_memory_tracing