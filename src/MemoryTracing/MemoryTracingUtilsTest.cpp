// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/substitute.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <string>

#include "GrpcProtos/capture.pb.h"
#include "MemoryTracing/MemoryTracingUtils.h"
#include "OrbitBase/Result.h"

using orbit_grpc_protos::CGroupMemoryUsage;
using orbit_grpc_protos::ProcessMemoryUsage;
using orbit_grpc_protos::SystemMemoryUsage;

namespace {

MATCHER_P(MemoryProtoEq, expected, "") {
  return arg.SerializeAsString() == expected.SerializeAsString();
}

}  // namespace

namespace orbit_memory_tracing {

TEST(MemoryUtils, UpdateSystemMemoryUsageFromMemInfo) {
  constexpr int kMemTotal = 16396576;
  constexpr int kMemFree = 11493816;
  constexpr int kMemAvailable = 14378752;
  constexpr int kBuffers = 71540;
  constexpr int kCached = 3042860;

  const std::string valid_meminfo =
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

  const std::string partial_meminfo = absl::Substitute(R"(MemTotal:       $0 kB
MemFree:        $1 kB
SwapCached:      0 kB)",
                                                       kMemTotal, kMemFree);

  const std::string empty_meminfo = "";

  {
    const SystemMemoryUsage expected_system_memory_usage = [] {
      SystemMemoryUsage system_memory_usage = CreateAndInitializeSystemMemoryUsage();
      system_memory_usage.set_total_kb(kMemTotal);
      system_memory_usage.set_free_kb(kMemFree);
      system_memory_usage.set_available_kb(kMemAvailable);
      system_memory_usage.set_buffers_kb(kBuffers);
      system_memory_usage.set_cached_kb(kCached);
      return system_memory_usage;
    }();
    SystemMemoryUsage system_memory_usage = CreateAndInitializeSystemMemoryUsage();
    ErrorMessageOr<void> updating_result =
        UpdateSystemMemoryUsageFromMemInfo(valid_meminfo, &system_memory_usage);
    EXPECT_FALSE(updating_result.has_error());
    EXPECT_THAT(system_memory_usage, MemoryProtoEq(expected_system_memory_usage));
  }

  {
    const SystemMemoryUsage expected_system_memory_usage = [] {
      SystemMemoryUsage system_memory_usage = CreateAndInitializeSystemMemoryUsage();
      system_memory_usage.set_total_kb(kMemTotal);
      system_memory_usage.set_free_kb(kMemFree);
      return system_memory_usage;
    }();
    SystemMemoryUsage system_memory_usage = CreateAndInitializeSystemMemoryUsage();
    ErrorMessageOr<void> updating_result =
        UpdateSystemMemoryUsageFromMemInfo(partial_meminfo, &system_memory_usage);
    EXPECT_FALSE(updating_result.has_error());
    EXPECT_THAT(system_memory_usage, MemoryProtoEq(expected_system_memory_usage));
  }

  {
    const SystemMemoryUsage expected_system_memory_usage = CreateAndInitializeSystemMemoryUsage();
    SystemMemoryUsage system_memory_usage = CreateAndInitializeSystemMemoryUsage();
    ErrorMessageOr<void> updating_result =
        UpdateSystemMemoryUsageFromMemInfo(empty_meminfo, &system_memory_usage);
    EXPECT_TRUE(updating_result.has_error());
    EXPECT_THAT(system_memory_usage, MemoryProtoEq(expected_system_memory_usage));
  }
}

TEST(MemoryUtils, UpdateSystemMemoryUsageFromVmStat) {
  constexpr int kPageFaults = 123456789;
  constexpr int kMajorPageFaults = 123456;

  const std::string valid_proc_vm_stat = absl::Substitute(R"(nr_free_pages 2258933
nr_zone_inactive_anon 655781
nr_zone_active_anon 265654
nr_zone_inactive_file 103608
nr_zone_active_file 682986
nr_zone_unevictable 14789
nr_zone_write_pending 504
nr_mlock 14789
nr_page_table_pages 14006
nr_bounce 0
nr_zspages 0
nr_free_cma 0
numa_hit 1640599383
numa_miss 0
numa_foreign 0
numa_interleave 61517
numa_local 1640599383
numa_other 0
nr_inactive_anon 655795
nr_active_anon 265654
nr_inactive_file 103608
nr_active_file 682986
nr_unevictable 14789
nr_slab_reclaimable 39573
nr_slab_unreclaimable 29913
nr_isolated_anon 0
nr_isolated_file 0
workingset_nodes 10052
workingset_refault_anon 482478
workingset_refault_file 4691743
workingset_activate_anon 83978
workingset_activate_file 3712979
workingset_restore_anon 31279
workingset_restore_file 2506434
workingset_nodereclaim 23964
nr_anon_pages 779841
nr_mapped 238243
nr_file_pages 882760
nr_dirty 480
nr_writeback 0
nr_writeback_temp 0
nr_shmem 66116
nr_shmem_hugepages 0
nr_shmem_pmdmapped 0
nr_file_hugepages 0
nr_file_pmdmapped 0
nr_anon_transparent_hugepages 755
nr_vmscan_write 1246151
nr_vmscan_immediate_reclaim 732
nr_dirtied 110747698
nr_written 96424883
nr_kernel_misc_reclaimable 0
nr_foll_pin_acquired 0
nr_foll_pin_released 0
nr_kernel_stack 39280
nr_dirty_threshold 600497
nr_dirty_background_threshold 299882
pgpgin 70153910
pgpgout 478359020
pswpin 482479
pswpout 1226100
pgalloc_dma 0
pgalloc_dma32 206502602
pgalloc_normal 2867571518
pgalloc_movable 0
allocstall_dma 0
allocstall_dma32 0
allocstall_normal 61
allocstall_movable 574
pgskip_dma 0
pgskip_dma32 0
pgskip_normal 255855
pgskip_movable 0
pgfree 3077305458
pgactivate 59489152
pgdeactivate 13444038
pglazyfree 176961
pgfault $0
pgmajfault $1
pglazyfreed 86974
pgrefill 14648260
pgreuse 150268511
pgsteal_kswapd 25809003
pgsteal_direct 109534
pgscan_kswapd 42547232
pgscan_direct 182478
pgscan_direct_throttle 0
pgscan_anon 16823270
pgscan_file 25906440
pgsteal_anon 1236888
pgsteal_file 24681649
zone_reclaim_failed 0
pginodesteal 7256
slabs_scanned 15016420
kswapd_inodesteal 8299045
kswapd_low_wmark_hit_quickly 3520
kswapd_high_wmark_hit_quickly 1113
pageoutrun 5198
pgrotated 1183212
drop_pagecache 0
drop_slab 0
oom_kill 0
numa_pte_updates 0
numa_huge_pte_updates 78
numa_hint_faults 0
numa_hint_faults_local 0
numa_pages_migrated 0
pgmigrate_success 835315
pgmigrate_fail 141734
thp_migration_success 0
thp_migration_fail 0
thp_migration_split 0
compact_migrate_scanned 22847132
compact_free_scanned 22310540
compact_isolated 1850479
compact_stall 209
compact_fail 7
compact_success 202
compact_daemon_wake 1419
compact_daemon_migrate_scanned 333848
compact_daemon_free_scanned 6526252
htlb_buddy_alloc_success 0
htlb_buddy_alloc_fail 0
unevictable_pgs_culled 207448
unevictable_pgs_scanned 0
unevictable_pgs_rescued 133162
unevictable_pgs_mlocked 160277
unevictable_pgs_munlocked 133138
unevictable_pgs_cleared 5564
unevictable_pgs_stranded 5534
thp_fault_alloc 2578050
thp_fault_fallback 2462
thp_fault_fallback_charge 0
thp_collapse_alloc 59381
thp_collapse_alloc_failed 2
thp_file_alloc 0
thp_file_fallback 0
thp_file_fallback_charge 0
thp_file_mapped 0
thp_split_page 1816
thp_split_page_failed 0
thp_deferred_split_page 224583
thp_split_pmd 660273
thp_split_pud 0
thp_zero_page_alloc 1
thp_zero_page_alloc_failed 0
thp_swpout 0
thp_swpout_fallback 782
balloon_inflate 209231935
balloon_deflate 209231935
balloon_migrate 3482
swap_ra 277950
swap_ra_hit 207052
nr_unstable 0)",
                                                          kPageFaults, kMajorPageFaults);

  const std::string partial_proc_vm_stat = absl::Substitute(R"(pgfault $0)", kPageFaults);

  const std::string empty_proc_vm_stat = "";

  {
    const SystemMemoryUsage expected_system_memory_usage = [] {
      SystemMemoryUsage system_memory_usage = CreateAndInitializeSystemMemoryUsage();
      system_memory_usage.set_pgfault(kPageFaults);
      system_memory_usage.set_pgmajfault(kMajorPageFaults);
      return system_memory_usage;
    }();
    SystemMemoryUsage system_memory_usage = CreateAndInitializeSystemMemoryUsage();
    ErrorMessageOr<void> updating_result =
        UpdateSystemMemoryUsageFromVmStat(valid_proc_vm_stat, &system_memory_usage);
    EXPECT_FALSE(updating_result.has_error());
    EXPECT_THAT(system_memory_usage, MemoryProtoEq(expected_system_memory_usage));
  }

  {
    const SystemMemoryUsage expected_system_memory_usage = [] {
      SystemMemoryUsage system_memory_usage = CreateAndInitializeSystemMemoryUsage();
      system_memory_usage.set_pgfault(kPageFaults);
      return system_memory_usage;
    }();
    SystemMemoryUsage system_memory_usage = CreateAndInitializeSystemMemoryUsage();
    ErrorMessageOr<void> updating_result =
        UpdateSystemMemoryUsageFromVmStat(partial_proc_vm_stat, &system_memory_usage);
    EXPECT_FALSE(updating_result.has_error());
    EXPECT_THAT(system_memory_usage, MemoryProtoEq(expected_system_memory_usage));
  }

  {
    const SystemMemoryUsage expected_system_memory_usage = CreateAndInitializeSystemMemoryUsage();
    SystemMemoryUsage system_memory_usage = CreateAndInitializeSystemMemoryUsage();
    ErrorMessageOr<void> updating_result =
        UpdateSystemMemoryUsageFromVmStat(empty_proc_vm_stat, &system_memory_usage);
    EXPECT_TRUE(updating_result.has_error());
    EXPECT_THAT(system_memory_usage, MemoryProtoEq(expected_system_memory_usage));
  }
}

TEST(MemoryUtils, UpdateProcessMemoryUsageFromProcessStat) {
  constexpr int kMinorPageFaults = 20;
  constexpr int kMajorPageFaults = 1;

  const std::string valid_process_stat = absl::Substitute(
      R"(9562 (TargetProcess) S 9561 9561 9561 0 -1 123456789 $0 3173 $1 0 7 18 1 7 20 0 10 0 123456789 123456789 2793 123456789 1 1 0 0 0 0 0 0 2 0 0 0 17 6 0 0 0 0 0 0 0 0 0 0 0 0 0)",
      kMinorPageFaults, kMajorPageFaults);
  const std::string partial_process_stat = R"(9562 (TargetProcess) S 9561 9561 9561)";
  const std::string empty_process_stat = "";

  {
    const ProcessMemoryUsage expected_process_memory_usage = [] {
      ProcessMemoryUsage process_memory_usage = CreateAndInitializeProcessMemoryUsage();
      process_memory_usage.set_minflt(kMinorPageFaults);
      process_memory_usage.set_majflt(kMajorPageFaults);
      return process_memory_usage;
    }();
    ProcessMemoryUsage process_memory_usage = CreateAndInitializeProcessMemoryUsage();
    ErrorMessageOr<void> updating_result =
        UpdateProcessMemoryUsageFromProcessStat(valid_process_stat, &process_memory_usage);
    EXPECT_FALSE(updating_result.has_error());
    EXPECT_THAT(process_memory_usage, MemoryProtoEq(expected_process_memory_usage));
  }

  {
    const ProcessMemoryUsage expected_process_memory_usage =
        CreateAndInitializeProcessMemoryUsage();
    ProcessMemoryUsage process_memory_usage = CreateAndInitializeProcessMemoryUsage();
    ErrorMessageOr<void> updating_result =
        UpdateProcessMemoryUsageFromProcessStat(partial_process_stat, &process_memory_usage);
    EXPECT_TRUE(updating_result.has_error());
    EXPECT_THAT(process_memory_usage, MemoryProtoEq(expected_process_memory_usage));
  }

  {
    const ProcessMemoryUsage expected_process_memory_usage =
        CreateAndInitializeProcessMemoryUsage();
    ProcessMemoryUsage process_memory_usage = CreateAndInitializeProcessMemoryUsage();
    ErrorMessageOr<void> updating_result =
        UpdateProcessMemoryUsageFromProcessStat(empty_process_stat, &process_memory_usage);
    EXPECT_TRUE(updating_result.has_error());
    EXPECT_THAT(process_memory_usage, MemoryProtoEq(expected_process_memory_usage));
  }
}

TEST(MemoryUtils, ExtractRssAnonFromProcessStatus) {
  constexpr int kRssAnonKb = 10264;

  const std::string valid_process_status = absl::Substitute(
      R"(Name:   bash
Umask:  0022
State:  S (sleeping)
Tgid:   17248
Ngid:   0
Pid:    17248
PPid:   17200
TracerPid:      0
Uid:    1000    1000    1000    1000
Gid:    100     100     100     100
FDSize: 256
Groups: 16 33 100
NStgid: 17248
NSpid:  17248
NSpgid: 17248
NSsid:  17200
VmPeak:     131168 kB
VmSize:     131168 kB
VmLck:           0 kB
VmPin:           0 kB
VmHWM:       13484 kB
VmRSS:       13484 kB
RssAnon:     $0 kB
RssFile:      3220 kB
RssShmem:        0 kB
VmData:      10332 kB
VmStk:         136 kB
VmExe:         992 kB
VmLib:        2104 kB
VmPTE:          76 kB
VmPMD:          12 kB
VmSwap:          0 kB
HugetlbPages:          0 kB
CoreDumping:    0
Threads:        1
SigQ:   0/3067
SigPnd: 0000000000000000
ShdPnd: 0000000000000000
SigBlk: 0000000000010000
SigIgn: 0000000000384004
SigCgt: 000000004b813efb
CapInh: 0000000000000000
CapPrm: 0000000000000000
CapEff: 0000000000000000
CapBnd: ffffffffffffffff
CapAmb: 0000000000000000
NoNewPrivs:     0
Seccomp:        0
Speculation_Store_Bypass:       vulnerable
Cpus_allowed:   00000001
Cpus_allowed_list:      0
Mems_allowed:   1
Mems_allowed_list:      0
voluntary_ctxt_switches:        150
nonvoluntary_ctxt_switches:     545)",
      kRssAnonKb);
  const std::string partial_process_status = R"(Name:   bash
Umask:  0022
State:  S (sleeping))";
  const std::string empty_process_status = "";

  {
    ErrorMessageOr<int64_t> extracting_result =
        ExtractRssAnonFromProcessStatus(valid_process_status);
    EXPECT_FALSE(extracting_result.has_error());
    EXPECT_EQ(extracting_result.value(), kRssAnonKb);
  }

  {
    ErrorMessageOr<int64_t> extracting_result =
        ExtractRssAnonFromProcessStatus(partial_process_status);
    EXPECT_TRUE(extracting_result.has_error());
  }

  {
    ErrorMessageOr<int64_t> extracting_result =
        ExtractRssAnonFromProcessStatus(empty_process_status);
    EXPECT_TRUE(extracting_result.has_error());
  }
}

TEST(MemoryUtil, GetProcessMemoryCGroupName) {
  const std::string c_group_name = "user.slice/user-1000.slice";

  const std::string valid_process_c_group = absl::Substitute(R"(10:memory:/$0
9:blkio:/user.slice/user-1000.slice
8:net_cls,net_prio:/
7:cpu,cpuacct:/user.slice/user-1000.slice
6:perf_event:/
5:freezer:/
4:cpuset:/
3:pids:/user.slice/user-1000.slice
2:devices:/user.slice/user-1000.slice
1:name=systemd:/user.slice/user-1000.slice/session-3.scope)",
                                                             c_group_name);

  const std::string partial_process_c_group = R"(3:pids:/user.slice/user-1000.slice
2:devices:/user.slice/user-1000.slice
1:name=systemd:/user.slice/user-1000.slice/session-3.scope)";

  const std::string empty_process_c_group = "";

  {
    std::string parsing_result = GetProcessMemoryCGroupName(valid_process_c_group);
    EXPECT_EQ(parsing_result, c_group_name);
  }

  {
    std::string parsing_result = GetProcessMemoryCGroupName(partial_process_c_group);
    EXPECT_TRUE(parsing_result.empty());
  }

  {
    std::string parsing_result = GetProcessMemoryCGroupName(empty_process_c_group);
    EXPECT_TRUE(parsing_result.empty());
  }
}

TEST(MemoryUtils, UpdateCGroupMemoryUsageFromMemoryStat) {
  constexpr int kRssInBytes = 245760;
  constexpr int kMappedFileInBytes = 1234;
  constexpr int kPageFaults = 1425;
  constexpr int kMajorPageFaults = 1;
  constexpr int kUnevictableInBytes = 0;
  constexpr int kInactiveAnonInBytes = 16384;
  constexpr int kActiveAnonInBytes = 253952;
  constexpr int kInactiveFileInBytes = 3678;
  constexpr int kActiveFileInBytes = 12288;

  const std::string valid_c_group_memory_status = absl::Substitute(
      R"(cache 36864
rss $0
rss_huge 0
shmem 0
mapped_file $1
dirty 135168
writeback 0
pgpgin 299
pgpgout 230
pgfault $2
pgmajfault $3
inactive_anon $5
active_anon $6
inactive_file $7
active_file $8
unevictable $4
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
total_pgmajfault 1
total_inactive_anon 16384
total_active_anon 253952
total_inactive_file 0
total_active_file 12288
total_unevictable 0)",
      kRssInBytes, kMappedFileInBytes, kPageFaults, kMajorPageFaults, kUnevictableInBytes,
      kInactiveAnonInBytes, kActiveAnonInBytes, kInactiveFileInBytes, kActiveFileInBytes);

  const std::string partial_c_group_memory_status = R"(cache 36864
rss_huge 0)";

  const std::string empty_c_group_memory_status = "";

  {
    const CGroupMemoryUsage expected_c_group_memory_usage = [] {
      CGroupMemoryUsage cgroup_memory_usage = CreateAndInitializeCGroupMemoryUsage();
      cgroup_memory_usage.set_rss_bytes(kRssInBytes);
      cgroup_memory_usage.set_mapped_file_bytes(kMappedFileInBytes);
      cgroup_memory_usage.set_pgfault(kPageFaults);
      cgroup_memory_usage.set_pgmajfault(kMajorPageFaults);
      cgroup_memory_usage.set_unevictable_bytes(kUnevictableInBytes);
      cgroup_memory_usage.set_inactive_anon_bytes(kInactiveAnonInBytes);
      cgroup_memory_usage.set_active_anon_bytes(kActiveAnonInBytes);
      cgroup_memory_usage.set_inactive_file_bytes(kInactiveFileInBytes);
      cgroup_memory_usage.set_active_file_bytes(kActiveFileInBytes);
      return cgroup_memory_usage;
    }();

    CGroupMemoryUsage cgroup_memory_usage = CreateAndInitializeCGroupMemoryUsage();
    ErrorMessageOr<void> updating_result =
        UpdateCGroupMemoryUsageFromMemoryStat(valid_c_group_memory_status, &cgroup_memory_usage);
    EXPECT_FALSE(updating_result.has_error());
    EXPECT_THAT(cgroup_memory_usage, MemoryProtoEq(expected_c_group_memory_usage));
  }

  {
    const CGroupMemoryUsage expected_c_group_memory_usage = CreateAndInitializeCGroupMemoryUsage();
    CGroupMemoryUsage cgroup_memory_usage = CreateAndInitializeCGroupMemoryUsage();
    ErrorMessageOr<void> updating_result =
        UpdateCGroupMemoryUsageFromMemoryStat(partial_c_group_memory_status, &cgroup_memory_usage);
    EXPECT_FALSE(updating_result.has_error());
    EXPECT_THAT(cgroup_memory_usage, MemoryProtoEq(expected_c_group_memory_usage));
  }

  {
    const CGroupMemoryUsage expected_c_group_memory_usage = CreateAndInitializeCGroupMemoryUsage();
    CGroupMemoryUsage cgroup_memory_usage = CreateAndInitializeCGroupMemoryUsage();
    ErrorMessageOr<void> updating_result =
        UpdateCGroupMemoryUsageFromMemoryStat(empty_c_group_memory_status, &cgroup_memory_usage);
    EXPECT_TRUE(updating_result.has_error());
    EXPECT_THAT(cgroup_memory_usage, MemoryProtoEq(expected_c_group_memory_usage));
  }
}

}  // namespace orbit_memory_tracing