// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/module.pb.h"
#include "LibunwindstackMaps.h"
#include "MockTracerListener.h"
#include "PerfEvent.h"
#include "Test/Path.h"
#include "UprobesFunctionCallManager.h"
#include "UprobesUnwindingVisitor.h"
#include "UprobesUnwindingVisitorTestCommon.h"

using ::testing::SaveArg;

namespace orbit_linux_tracing {

namespace {

class UprobesUnwindingVisitorMmapTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ON_CALL(maps_, Find).WillByDefault([this](uint64_t pc) { return real_maps_->Find(pc); });
    ON_CALL(maps_, Get).WillByDefault([this]() { return real_maps_->Get(); });
    ON_CALL(maps_, AddAndSort)
        .WillByDefault([this](uint64_t start, uint64_t end, uint64_t offset, uint64_t flags,
                              std::string_view name) {
          return real_maps_->AddAndSort(start, end, offset, flags, name);
        });
  }

  static constexpr pid_t kPid = 42;

  MockTracerListener listener_;
  MockLibunwindstackMaps maps_;
  UprobesUnwindingVisitor visitor_{&listener_,
                                   &function_call_manager_,
                                   &return_address_manager_,
                                   &maps_,
                                   &unwinder_,
                                   &leaf_function_call_manager_,
                                   /*user_space_instrumentation_addresses=*/nullptr,
                                   /*absolute_address_to_size_of_functions_to_stop_at=*/nullptr};

 private:
  UprobesFunctionCallManager function_call_manager_;
  MockUprobesReturnAddressManager return_address_manager_{
      /*user_space_instrumentation_addresses=*/nullptr};
  std::unique_ptr<LibunwindstackMaps> real_maps_ = LibunwindstackMaps::ParseMaps("");
  MockLibunwindstackUnwinder unwinder_;
  MockLeafFunctionCallManager leaf_function_call_manager_{128};
};

constexpr uint64_t kTargetFpFileSize = 27824;
constexpr const char* kTargetFpBuildId = "d7e2447f79faa88528dd0d130ac7cc5f168ca090";
constexpr uint64_t kTargetFpLoadBias = 0;
constexpr uint64_t kTargetFpExecutableSegmentOffset = 0x1000;

static void VerifyObjectSegmentsForTargetFp(
    const google::protobuf::RepeatedPtrField<orbit_grpc_protos::ModuleInfo::ObjectSegment>&
        segments) {
  ASSERT_EQ(segments.size(), 4);
  // Simple sanity check, don't verify every single segment.
  EXPECT_EQ(segments[0].offset_in_file(), 0);
  EXPECT_EQ(segments[0].size_in_file(), 0xa48);
  EXPECT_EQ(segments[0].address(), 0);
  EXPECT_EQ(segments[0].size_in_memory(), 0xa48);
}

constexpr uint64_t kLibtestDllFileSize = 96441;
constexpr const char* kLibtestDllBuildId = "";
constexpr uint64_t kLibtestDllImageBase = 0x62640000;
constexpr uint64_t kLibtestDllBaseOfCode = 0x1000;

static void VerifyObjectSegmentsForLibtestDll(
    const google::protobuf::RepeatedPtrField<orbit_grpc_protos::ModuleInfo::ObjectSegment>&
        segments) {
  ASSERT_EQ(segments.size(), 19);
  EXPECT_EQ(segments[0].offset_in_file(), 0x600);
  EXPECT_EQ(segments[0].size_in_file(), 0x1400);
  EXPECT_EQ(segments[0].address(), kLibtestDllImageBase + 0x1000);
  EXPECT_EQ(segments[0].size_in_memory(), 0x1338);
}

}  // namespace

TEST_F(UprobesUnwindingVisitorMmapTest,
       VisitMmapPerfEventUpdatesLibunwindstackMapsAndSendsModuleUpdates) {
  // 7f4b0c7ab000-7f4b0c845000 r-xp 00000000 00:00 0
  // Anonymous executable mapping.
  MmapPerfEvent anon_mmap_event{
      .timestamp = 1,
      .data =
          {
              .address = 0x7f4b0c7ab000,
              .length = 0x9A000,
              .page_offset = 0,
              .filename = "",
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x7f4b0c7ab000, 0x7f4b0c845000, 0, PROT_READ | PROT_EXEC, ""))
      .Times(1);
  EXPECT_CALL(maps_, Find(0x7f4b0c7ab000)).Times(1);
  PerfEvent(std::move(anon_mmap_event)).Accept(&visitor_);

  // 7fffffffe000-7ffffffff000 --xp 00000000 00:00 0    [uprobes]
  // Special anonymous executable mapping.
  MmapPerfEvent special_mmap_event{
      .timestamp = 2,
      .data =
          {
              .address = 0x7fffffffe000,
              .length = 0x1000,
              .page_offset = 0,
              .filename = "[uprobes]",
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_,
              AddAndSort(0x7fffffffe000, 0x7ffffffff000, 0, PROT_READ | PROT_EXEC, "[uprobes]"))
      .Times(1);
  PerfEvent(std::move(special_mmap_event)).Accept(&visitor_);

  const std::string test_binary_path = (orbit_test::GetTestdataDir() / "target_fp").string();

  // 55bf53c22000-55bf53c24000 r-xp 00001000 fe:00 60425802    /path/to/target_fp
  // File-backed executable mapping.
  MmapPerfEvent file_mmap_event{
      .timestamp = 3,
      .data =
          {
              .address = 0x55bf53c22000,
              .length = 0x2000,
              .page_offset = 0x1000,
              .filename = test_binary_path,
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x55bf53c22000, 0x55bf53c24000, 0x1000, PROT_READ | PROT_EXEC,
                                test_binary_path))
      .Times(1);
  EXPECT_CALL(maps_, Find(0x55bf53c22000)).Times(1);
  orbit_grpc_protos::ModuleUpdateEvent actual_module_update;
  EXPECT_CALL(listener_, OnModuleUpdate).Times(1).WillOnce(SaveArg<0>(&actual_module_update));
  PerfEvent(std::move(file_mmap_event)).Accept(&visitor_);
  EXPECT_EQ(actual_module_update.pid(), kPid);
  EXPECT_EQ(actual_module_update.timestamp_ns(), 3);
  EXPECT_EQ(actual_module_update.module().name(), "target_fp");
  EXPECT_EQ(actual_module_update.module().file_path(), test_binary_path);
  EXPECT_EQ(actual_module_update.module().file_size(), kTargetFpFileSize);
  EXPECT_EQ(actual_module_update.module().address_start(), 0x55bf53c22000);
  EXPECT_EQ(actual_module_update.module().address_end(), 0x55bf53c24000);
  EXPECT_EQ(actual_module_update.module().build_id(), kTargetFpBuildId);
  EXPECT_EQ(actual_module_update.module().load_bias(), kTargetFpLoadBias);
  EXPECT_EQ(actual_module_update.module().executable_segment_offset(),
            kTargetFpExecutableSegmentOffset);
  EXPECT_EQ(actual_module_update.module().soname(), "");
  EXPECT_EQ(actual_module_update.module().object_file_type(),
            orbit_grpc_protos::ModuleInfo::kElfFile);
  VerifyObjectSegmentsForTargetFp(actual_module_update.module().object_segments());

  // 55bf53c24000-55bf53c25000 r--p 00003000 fe:00 60425802    /path/to/target_fp
  // File-backed non-executable mapping.
  MmapPerfEvent file_mmap_data_event{
      .timestamp = 4,
      .data =
          {
              .address = 0x55bf53c24000,
              .length = 0x1000,
              .page_offset = 0x3000,
              .filename = test_binary_path,
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_,
              AddAndSort(0x55bf53c24000, 0x55bf53c25000, 0x3000, PROT_READ, test_binary_path))
      .Times(1);
  PerfEvent(std::move(file_mmap_data_event)).Accept(&visitor_);

  // 7f4b0cabe000-7f4b0cad5000 r-xp 00003000 fe:00 50336180    /path/to/nothing
  // File-backed executable mapping, but the file doesn't exist.
  MmapPerfEvent bad_file_mmap_event{
      .timestamp = 5,
      .data =
          {
              .address = 0x7f4b0cabe000,
              .length = 0x17000,
              .page_offset = 0x3000,
              .filename = "/path/to/nothing",
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x7f4b0cabe000, 0x7f4b0cad5000, 0x3000, PROT_READ | PROT_EXEC,
                                "/path/to/nothing"))
      .Times(1);
  EXPECT_CALL(maps_, Find(0x7f4b0cabe000)).Times(1);
  PerfEvent(std::move(bad_file_mmap_event)).Accept(&visitor_);
}

TEST_F(UprobesUnwindingVisitorMmapTest,
       VisitMmapPerfEventSendsModuleUpdatesForElfWithTextSplitAcrossTwoMaps) {
  const std::string test_binary_path = (orbit_test::GetTestdataDir() / "target_fp").string();

  // 56224057e000-56224057f000 r--p 00000000 fe:00 60425802    /path/to/target_fp    <--
  MmapPerfEvent segment1_mmap_data_event{
      .timestamp = 1,
      .data =
          {
              .address = 0x56224057e000,
              .length = 0x1000,
              .page_offset = 0,
              .filename = test_binary_path,
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x56224057e000, 0x56224057f000, 0, PROT_READ, test_binary_path))
      .Times(1);
  PerfEvent(std::move(segment1_mmap_data_event)).Accept(&visitor_);

  // 56224057e000-56224057f000 r--p 00000000 fe:00 60425802    /path/to/target_fp
  // 56224057f000-562240580000 r-xp 00001000 fe:00 60425802    /path/to/target_fp    <--
  MmapPerfEvent segment2_part1_mmap_event{
      .timestamp = 2,
      .data =
          {
              .address = 0x56224057f000,
              .length = 0x1000,
              .page_offset = 0x1000,
              .filename = test_binary_path,
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x56224057f000, 0x562240580000, 0x1000, PROT_READ | PROT_EXEC,
                                test_binary_path))
      .Times(1);
  EXPECT_CALL(maps_, Find(0x56224057f000)).Times(1);
  orbit_grpc_protos::ModuleUpdateEvent actual_module_update;
  EXPECT_CALL(listener_, OnModuleUpdate).Times(1).WillOnce(SaveArg<0>(&actual_module_update));
  PerfEvent(std::move(segment2_part1_mmap_event)).Accept(&visitor_);
  EXPECT_EQ(actual_module_update.pid(), kPid);
  EXPECT_EQ(actual_module_update.timestamp_ns(), 2);
  EXPECT_EQ(actual_module_update.module().name(), "target_fp");
  EXPECT_EQ(actual_module_update.module().file_path(), test_binary_path);
  EXPECT_EQ(actual_module_update.module().file_size(), kTargetFpFileSize);
  EXPECT_EQ(actual_module_update.module().address_start(), 0x56224057f000);
  EXPECT_EQ(actual_module_update.module().address_end(), 0x562240580000);
  EXPECT_EQ(actual_module_update.module().build_id(), kTargetFpBuildId);
  EXPECT_EQ(actual_module_update.module().load_bias(), kTargetFpLoadBias);
  EXPECT_EQ(actual_module_update.module().executable_segment_offset(),
            kTargetFpExecutableSegmentOffset);
  EXPECT_EQ(actual_module_update.module().soname(), "");
  EXPECT_EQ(actual_module_update.module().object_file_type(),
            orbit_grpc_protos::ModuleInfo::kElfFile);
  VerifyObjectSegmentsForTargetFp(actual_module_update.module().object_segments());

  // 56224057e000-56224057f000 r--p 00000000 fe:00 60425802    /path/to/target_fp
  // 56224057f000-562240580000 r-xp 00001000 fe:00 60425802    /path/to/target_fp
  // 562240580000-562240581000 r-xp 00002000 fe:00 60425802    /path/to/target_fp    <--
  MmapPerfEvent segment2_part2_mmap_event{
      .timestamp = 3,
      .data =
          {
              .address = 0x562240580000,
              .length = 0x1000,
              .page_offset = 0x2000,
              .filename = test_binary_path,
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x562240580000, 0x562240581000, 0x2000, PROT_READ | PROT_EXEC,
                                test_binary_path))
      .Times(1);
  EXPECT_CALL(maps_, Find(0x562240580000)).Times(1);
  EXPECT_CALL(listener_, OnModuleUpdate).Times(1).WillOnce(SaveArg<0>(&actual_module_update));
  PerfEvent(std::move(segment2_part2_mmap_event)).Accept(&visitor_);
  EXPECT_EQ(actual_module_update.pid(), kPid);
  EXPECT_EQ(actual_module_update.timestamp_ns(), 3);
  EXPECT_EQ(actual_module_update.module().name(), "target_fp");
  EXPECT_EQ(actual_module_update.module().file_path(), test_binary_path);
  EXPECT_EQ(actual_module_update.module().file_size(), kTargetFpFileSize);
  EXPECT_EQ(actual_module_update.module().address_start(),
            0x56224057f000);  // Starts at the previous mapping, as intended.
  EXPECT_EQ(actual_module_update.module().address_end(), 0x562240581000);
  EXPECT_EQ(actual_module_update.module().build_id(), kTargetFpBuildId);
  EXPECT_EQ(actual_module_update.module().load_bias(), kTargetFpLoadBias);
  EXPECT_EQ(actual_module_update.module().executable_segment_offset(),
            kTargetFpExecutableSegmentOffset);
  EXPECT_EQ(actual_module_update.module().soname(), "");
  EXPECT_EQ(actual_module_update.module().object_file_type(),
            orbit_grpc_protos::ModuleInfo::kElfFile);
  VerifyObjectSegmentsForTargetFp(actual_module_update.module().object_segments());

  // 56224057e000-56224057f000 r--p 00000000 fe:00 60425802    /path/to/target_fp
  // 56224057f000-562240580000 r-xp 00001000 fe:00 60425802    /path/to/target_fp
  // 562240580000-562240581000 r-xp 00002000 fe:00 60425802    /path/to/target_fp
  // 562240581000-562240583000 r--p 00003000 fe:00 60425802    /path/to/target_fp    <--
  MmapPerfEvent segment3_mmap_data_event{
      .timestamp = 4,
      .data =
          {
              .address = 0x562240581000,
              .length = 0x2000,
              .page_offset = 0x3000,
              .filename = test_binary_path,
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_,
              AddAndSort(0x562240581000, 0x562240583000, 0x3000, PROT_READ, test_binary_path))
      .Times(1);
  PerfEvent(std::move(segment3_mmap_data_event)).Accept(&visitor_);

  // 56224057e000-56224057f000 r--p 00000000 fe:00 60425802    /path/to/target_fp
  // 56224057f000-562240580000 r-xp 00001000 fe:00 60425802    /path/to/target_fp
  // 562240580000-562240581000 r-xp 00002000 fe:00 60425802    /path/to/target_fp
  // 562240581000-562240583000 r--p 00003000 fe:00 60425802    /path/to/target_fp
  // 562240583000-562240584000 rw-p 00004000 fe:00 60425802    /path/to/target_fp    <--
  MmapPerfEvent segment4_mmap_data_event{
      .timestamp = 5,
      .data =
          {
              .address = 0x562240583000,
              .length = 0x1000,
              .page_offset = 0x4000,
              .filename = test_binary_path,
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_,
              AddAndSort(0x562240583000, 0x562240584000, 0x4000, PROT_READ, test_binary_path))
      .Times(1);
  PerfEvent(std::move(segment4_mmap_data_event)).Accept(&visitor_);
}

TEST_F(UprobesUnwindingVisitorMmapTest,
       VisitMmapPerfEventSendsModuleUpdatesForPeTextSectionInAnonExecMap) {
  const std::string libtest_path = (orbit_test::GetTestdataDir() / "libtest.dll").string();

  // 100000-101000 r--p 00000000 01:02 42    /path/to/libtest.dll
  MmapPerfEvent headers_mmap_data_event{
      .timestamp = 1,
      .data =
          {
              .address = 0x100000,
              .length = 0x1000,
              .page_offset = 0,
              .filename = libtest_path,
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x100000, 0x101000, 0, PROT_READ, libtest_path)).Times(1);
  PerfEvent(std::move(headers_mmap_data_event)).Accept(&visitor_);

  // 101000-103000 r-xp 00000000 00:00 0
  MmapPerfEvent text_mmap_event{
      .timestamp = 2,
      .data =
          {
              .address = 0x101000,
              .length = 0x2000,
              .page_offset = 0,
              .filename = "",
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x101000, 0x103000, 0, PROT_READ | PROT_EXEC, "")).Times(1);
  EXPECT_CALL(maps_, Find(0x101000)).Times(1);
  orbit_grpc_protos::ModuleUpdateEvent actual_module_update;
  EXPECT_CALL(listener_, OnModuleUpdate).Times(1).WillOnce(SaveArg<0>(&actual_module_update));
  PerfEvent(std::move(text_mmap_event)).Accept(&visitor_);
  EXPECT_EQ(actual_module_update.pid(), kPid);
  EXPECT_EQ(actual_module_update.timestamp_ns(), 2);
  EXPECT_EQ(actual_module_update.module().name(), "libtest.dll");
  EXPECT_EQ(actual_module_update.module().file_path(), libtest_path);
  EXPECT_EQ(actual_module_update.module().file_size(), kLibtestDllFileSize);
  EXPECT_EQ(actual_module_update.module().address_start(), 0x101000);
  EXPECT_EQ(actual_module_update.module().address_end(), 0x103000);
  EXPECT_EQ(actual_module_update.module().build_id(), kLibtestDllBuildId);
  EXPECT_EQ(actual_module_update.module().load_bias(), kLibtestDllImageBase);
  EXPECT_EQ(actual_module_update.module().executable_segment_offset(), kLibtestDllBaseOfCode);
  EXPECT_EQ(actual_module_update.module().soname(), "");
  EXPECT_EQ(actual_module_update.module().object_file_type(),
            orbit_grpc_protos::ModuleInfo::kCoffFile);
  VerifyObjectSegmentsForLibtestDll(actual_module_update.module().object_segments());
}

TEST_F(UprobesUnwindingVisitorMmapTest,
       VisitMmapPerfEventSendsModuleUpdatesForPeExecutableSectionsInBothFileAndAnonMaps) {
  const std::string libtest_path = (orbit_test::GetTestdataDir() / "libtest.dll").string();

  // 100000-101000 r--p 00000000 01:02 42    /path/to/libtest.dll    <--
  MmapPerfEvent mmap_data_event1{
      .timestamp = 1,
      .data =
          {
              .address = 0x100000,
              .length = 0x1000,
              .page_offset = 0,
              .filename = libtest_path,
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x100000, 0x101000, 0, PROT_READ, libtest_path)).Times(1);
  PerfEvent(std::move(mmap_data_event1)).Accept(&visitor_);

  // 100000-101000 r--p 00000000 01:02 42    /path/to/libtest.dll
  // 101000-102000 r-xp 00000000 00:00 0                             <--
  MmapPerfEvent mmap_event2{
      .timestamp = 2,
      .data =
          {
              .address = 0x101000,
              .length = 0x1000,
              .page_offset = 0,
              .filename = "",
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x101000, 0x102000, 0, PROT_READ | PROT_EXEC, "")).Times(1);
  EXPECT_CALL(maps_, Find(0x101000)).Times(1);
  orbit_grpc_protos::ModuleUpdateEvent actual_module_update;
  EXPECT_CALL(listener_, OnModuleUpdate).Times(1).WillOnce(SaveArg<0>(&actual_module_update));
  PerfEvent(std::move(mmap_event2)).Accept(&visitor_);
  EXPECT_EQ(actual_module_update.pid(), kPid);
  EXPECT_EQ(actual_module_update.timestamp_ns(), 2);
  EXPECT_EQ(actual_module_update.module().name(), "libtest.dll");
  EXPECT_EQ(actual_module_update.module().file_path(), libtest_path);
  EXPECT_EQ(actual_module_update.module().file_size(), kLibtestDllFileSize);
  EXPECT_EQ(actual_module_update.module().address_start(), 0x101000);
  EXPECT_EQ(actual_module_update.module().address_end(), 0x102000);
  EXPECT_EQ(actual_module_update.module().build_id(), kLibtestDllBuildId);
  EXPECT_EQ(actual_module_update.module().load_bias(), kLibtestDllImageBase);
  EXPECT_EQ(actual_module_update.module().executable_segment_offset(), kLibtestDllBaseOfCode);
  EXPECT_EQ(actual_module_update.module().soname(), "");
  EXPECT_EQ(actual_module_update.module().object_file_type(),
            orbit_grpc_protos::ModuleInfo::kCoffFile);
  VerifyObjectSegmentsForLibtestDll(actual_module_update.module().object_segments());

  // 100000-101000 r--p 00000000 01:02 42    /path/to/libtest.dll
  // 101000-102000 r-xp 00000000 00:00 0
  // 102000-103000 r-xp 00002000 01:02 42    /path/to/libtest.dll    <--
  MmapPerfEvent mmap_event3{
      .timestamp = 3,
      .data =
          {
              .address = 0x102000,
              .length = 0x1000,
              .page_offset = 0x2000,
              .filename = libtest_path,
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x102000, 0x103000, 0x2000, PROT_READ | PROT_EXEC, libtest_path))
      .Times(1);
  EXPECT_CALL(maps_, Find(0x102000)).Times(1);
  EXPECT_CALL(listener_, OnModuleUpdate).Times(1).WillOnce(SaveArg<0>(&actual_module_update));
  PerfEvent(std::move(mmap_event3)).Accept(&visitor_);
  EXPECT_EQ(actual_module_update.pid(), kPid);
  EXPECT_EQ(actual_module_update.timestamp_ns(), 3);
  EXPECT_EQ(actual_module_update.module().name(), "libtest.dll");
  EXPECT_EQ(actual_module_update.module().file_path(), libtest_path);
  EXPECT_EQ(actual_module_update.module().file_size(), kLibtestDllFileSize);
  EXPECT_EQ(actual_module_update.module().address_start(),
            0x101000);  // Also includes the previous mapping, as intended.
  EXPECT_EQ(actual_module_update.module().address_end(), 0x103000);
  EXPECT_EQ(actual_module_update.module().build_id(), kLibtestDllBuildId);
  EXPECT_EQ(actual_module_update.module().load_bias(), kLibtestDllImageBase);
  EXPECT_EQ(actual_module_update.module().executable_segment_offset(), kLibtestDllBaseOfCode);
  EXPECT_EQ(actual_module_update.module().soname(), "");
  EXPECT_EQ(actual_module_update.module().object_file_type(),
            orbit_grpc_protos::ModuleInfo::kCoffFile);
  VerifyObjectSegmentsForLibtestDll(actual_module_update.module().object_segments());

  // 100000-101000 r--p 00000000 01:02 42    /path/to/libtest.dll
  // 101000-102000 r-xp 00000000 00:00 0
  // 102000-103000 r-xp 00002000 01:02 42    /path/to/libtest.dll
  // 103000-104000 r--p 00003000 01:02 42    /path/to/libtest.dll    <--
  MmapPerfEvent mmap_data_event4{
      .timestamp = 4,
      .data =
          {
              .address = 0x103000,
              .length = 0x1000,
              .page_offset = 0x3000,
              .filename = libtest_path,
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x103000, 0x104000, 0x3000, PROT_READ, libtest_path)).Times(1);
  PerfEvent(std::move(mmap_data_event4)).Accept(&visitor_);

  // 100000-101000 r--p 00000000 01:02 42    /path/to/libtest.dll
  // 101000-102000 r-xp 00000000 00:00 0
  // 102000-103000 r-xp 00002000 01:02 42    /path/to/libtest.dll
  // 103000-104000 r--p 00003000 01:02 42    /path/to/libtest.dll
  // 104000-105000 r-xp 00000000 00:00 0                             <--
  MmapPerfEvent mmap_event5{
      .timestamp = 5,
      .data =
          {
              .address = 0x104000,
              .length = 0x1000,
              .page_offset = 0,
              .filename = "",
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x104000, 0x105000, 0, PROT_READ | PROT_EXEC, "")).Times(1);
  EXPECT_CALL(maps_, Find(0x104000)).Times(1);
  EXPECT_CALL(listener_, OnModuleUpdate).Times(1).WillOnce(SaveArg<0>(&actual_module_update));
  PerfEvent(std::move(mmap_event5)).Accept(&visitor_);
  EXPECT_EQ(actual_module_update.pid(), kPid);
  EXPECT_EQ(actual_module_update.timestamp_ns(), 5);
  EXPECT_EQ(actual_module_update.module().name(), "libtest.dll");
  EXPECT_EQ(actual_module_update.module().file_path(), libtest_path);
  EXPECT_EQ(actual_module_update.module().file_size(), kLibtestDllFileSize);
  EXPECT_EQ(actual_module_update.module().address_start(),
            0x101000);  // Also includes the previous two executable mappings, as intended.
  EXPECT_EQ(actual_module_update.module().address_end(), 0x105000);
  EXPECT_EQ(actual_module_update.module().build_id(), kLibtestDllBuildId);
  EXPECT_EQ(actual_module_update.module().load_bias(), kLibtestDllImageBase);
  EXPECT_EQ(actual_module_update.module().executable_segment_offset(), kLibtestDllBaseOfCode);
  EXPECT_EQ(actual_module_update.module().soname(), "");
  EXPECT_EQ(actual_module_update.module().object_file_type(),
            orbit_grpc_protos::ModuleInfo::kCoffFile);
  VerifyObjectSegmentsForLibtestDll(actual_module_update.module().object_segments());
}

// This test simulates the sequence of PERF_RECORD_MMAPs caused by Wine's virtual_map_image
// (https://github.com/wine-mirror/wine/blob/dfeded6460ce067fe1c0540306c2964a170bed2a/dlls/ntdll/unix/virtual.c#L2467)
// on a PE with SizeOfImage is 0x20000, BaseOfCode is 0x1000, and six sections, the first of
// which executable, and the third of which writeable. The sequence of events was also deduced by
// observing the events generated by Wine mapping triangle.exe.
TEST_F(UprobesUnwindingVisitorMmapTest, VisitMmapPerfEventSendsModuleUpdatesForPeMappedByWine) {
  const std::string libtest_path = (orbit_test::GetTestdataDir() / "libtest.dll").string();

  // The following PERF_RECORD_MMAP(s) are caused by:
  // map_view
  // https://github.com/wine-mirror/wine/blob/dfeded6460ce067fe1c0540306c2964a170bed2a/dlls/ntdll/unix/virtual.c#L1936

  // 100000-120000 rwxp 00000000 00:00 0    <--
  MmapPerfEvent whole_file_mmap_event{
      .timestamp = 1,
      .data =
          {
              .address = 0x100000,
              .length = 0x20000,
              .page_offset = 0,
              .filename = "",
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x100000, 0x120000, 0, PROT_READ | PROT_EXEC, "")).Times(1);
  EXPECT_CALL(maps_, Find(0x100000)).Times(1);
  PerfEvent(std::move(whole_file_mmap_event)).Accept(&visitor_);

  // The following PERF_RECORD_MMAP(s) are caused by:
  // map_image_into_view's /* map the header */
  // https://github.com/wine-mirror/wine/blob/dfeded6460ce067fe1c0540306c2964a170bed2a/dlls/ntdll/unix/virtual.c#L2238

  // 100000-101000 rwxp 00000000 01:02 42    /path/to/libtest.dll    <--
  // 101000-120000 rwxp 00000000 00:00 0
  MmapPerfEvent headers_mmap_event{
      .timestamp = 2,
      .data =
          {
              .address = 0x100000,
              .length = 0x1000,
              .page_offset = 0,
              .filename = libtest_path,
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x100000, 0x101000, 0, PROT_READ | PROT_EXEC, libtest_path))
      .Times(1);
  EXPECT_CALL(maps_, Find(0x100000)).Times(1);
  orbit_grpc_protos::ModuleUpdateEvent actual_module_update;
  EXPECT_CALL(listener_, OnModuleUpdate).Times(1).WillOnce(SaveArg<0>(&actual_module_update));
  PerfEvent(std::move(headers_mmap_event)).Accept(&visitor_);
  EXPECT_EQ(actual_module_update.pid(), kPid);
  EXPECT_EQ(actual_module_update.timestamp_ns(), 2);
  EXPECT_EQ(actual_module_update.module().name(), "libtest.dll");
  EXPECT_EQ(actual_module_update.module().file_path(), libtest_path);
  EXPECT_EQ(actual_module_update.module().file_size(), kLibtestDllFileSize);
  EXPECT_EQ(actual_module_update.module().address_start(), 0x100000);
  EXPECT_EQ(actual_module_update.module().address_end(),
            0x120000);  // Also includes the next mapping, as intended.
  EXPECT_EQ(actual_module_update.module().build_id(), kLibtestDllBuildId);
  EXPECT_EQ(actual_module_update.module().load_bias(), kLibtestDllImageBase);
  EXPECT_EQ(actual_module_update.module().executable_segment_offset(), kLibtestDllBaseOfCode);
  EXPECT_EQ(actual_module_update.module().soname(), "");
  EXPECT_EQ(actual_module_update.module().object_file_type(),
            orbit_grpc_protos::ModuleInfo::kCoffFile);
  VerifyObjectSegmentsForLibtestDll(actual_module_update.module().object_segments());

  // The following PERF_RECORD_MMAP(s) are caused by:
  // map_image_into_view's /* map all the sections */
  // https://github.com/wine-mirror/wine/blob/dfeded6460ce067fe1c0540306c2964a170bed2a/dlls/ntdll/unix/virtual.c#L2288

  // 100000-101000 rwxp 00000000 01:02 42    /path/to/libtest.dll
  // 101000-103000 rw-p 00000000 00:00 0                             <--
  // 103000-120000 rwxp 00000000 00:00 0
  MmapPerfEvent section1_mmap_data_event{
      .timestamp = 3,
      .data =
          {
              .address = 0x101000,
              .length = 0x2000,
              .page_offset = 0,
              .filename = "",
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x101000, 0x103000, 0, PROT_READ, "")).Times(1);
  PerfEvent(std::move(section1_mmap_data_event)).Accept(&visitor_);

  // 100000-101000 rwxp 00000000 01:02 42    /path/to/libtest.dll
  // 101000-105000 rw-p 00000000 00:00 0                             <--
  // 105000-120000 rwxp 00000000 00:00 0
  MmapPerfEvent section2_mmap_data_event{
      .timestamp = 4,
      .data =
          {
              .address = 0x101000,
              .length = 0x4000,
              .page_offset = 0,
              .filename = "",
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x101000, 0x105000, 0, PROT_READ, "")).Times(1);
  PerfEvent(std::move(section2_mmap_data_event)).Accept(&visitor_);

  // 100000-101000 rwxp 00000000 01:02 42    /path/to/libtest.dll
  // 101000-106000 rw-p 00000000 00:00 0                             <--
  // 106000-120000 rwxp 00000000 00:00 0
  MmapPerfEvent section3_mmap_data_event{
      .timestamp = 5,
      .data =
          {
              .address = 0x101000,
              .length = 0x5000,
              .page_offset = 0,
              .filename = "",
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x101000, 0x106000, 0, PROT_READ, "")).Times(1);
  PerfEvent(std::move(section3_mmap_data_event)).Accept(&visitor_);

  // 100000-101000 rwxp 00000000 01:02 42    /path/to/libtest.dll
  // 101000-107000 rw-p 00000000 00:00 0                             <--
  // 107000-120000 rwxp 00000000 00:00 0
  MmapPerfEvent section4_mmap_data_event{
      .timestamp = 6,
      .data =
          {
              .address = 0x101000,
              .length = 0x6000,
              .page_offset = 0,
              .filename = "",
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x101000, 0x107000, 0, PROT_READ, "")).Times(1);
  PerfEvent(std::move(section4_mmap_data_event)).Accept(&visitor_);

  // 100000-101000 rwxp 00000000 01:02 42    /path/to/libtest.dll
  // 101000-11f000 rw-p 00000000 00:00 0                             <--
  // 11f000-120000 rwxp 00000000 00:00 0
  MmapPerfEvent section5_mmap_data_event{
      .timestamp = 7,
      .data =
          {
              .address = 0x101000,
              .length = 0x1e000,
              .page_offset = 0,
              .filename = "",
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x101000, 0x11f000, 0, PROT_READ, "")).Times(1);
  PerfEvent(std::move(section5_mmap_data_event)).Accept(&visitor_);

  // 100000-101000 rwxp 00000000 01:02 42    /path/to/libtest.dll
  // 101000-120000 rw-p 00000000 00:00 0                             <--
  MmapPerfEvent section6_mmap_data_event{
      .timestamp = 8,
      .data =
          {
              .address = 0x101000,
              .length = 0x1f000,
              .page_offset = 0,
              .filename = "",
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x101000, 0x120000, 0, PROT_READ, "")).Times(1);
  PerfEvent(std::move(section6_mmap_data_event)).Accept(&visitor_);

  // The following PERF_RECORD_MMAP(s) are caused by:
  // map_image_into_view's /* set the image protections */
  // https://github.com/wine-mirror/wine/blob/dfeded6460ce067fe1c0540306c2964a170bed2a/dlls/ntdll/unix/virtual.c#L2378

  // 100000-101000 r--p 00000000 01:02 42    /path/to/libtest.dll    <--
  // 101000-120000 rw-p 00000000 00:00 0
  MmapPerfEvent headers_protection_mmap_data_event{
      .timestamp = 9,
      .data =
          {
              .address = 0x100000,
              .length = 0x1000,
              .page_offset = 0,
              .filename = libtest_path,
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x100000, 0x101000, 0, PROT_READ, libtest_path)).Times(1);
  PerfEvent(std::move(headers_protection_mmap_data_event)).Accept(&visitor_);

  // 100000-101000 r--p 00000000 01:02 42    /path/to/libtest.dll
  // 101000-103000 r-xp 00000000 00:00 0                             <--
  // 103000-120000 rw-p 00000000 00:00 0
  MmapPerfEvent section1_protection_mmap_event{
      .timestamp = 10,
      .data =
          {
              .address = 0x101000,
              .length = 0x2000,
              .page_offset = 0,
              .filename = "",
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x101000, 0x103000, 0, PROT_READ | PROT_EXEC, "")).Times(1);
  EXPECT_CALL(maps_, Find(0x101000)).Times(1);
  EXPECT_CALL(listener_, OnModuleUpdate).Times(1).WillOnce(SaveArg<0>(&actual_module_update));
  PerfEvent(std::move(section1_protection_mmap_event)).Accept(&visitor_);
  EXPECT_EQ(actual_module_update.pid(), kPid);
  EXPECT_EQ(actual_module_update.timestamp_ns(), 10);
  EXPECT_EQ(actual_module_update.module().name(), "libtest.dll");
  EXPECT_EQ(actual_module_update.module().file_path(), libtest_path);
  EXPECT_EQ(actual_module_update.module().file_size(), kLibtestDllFileSize);
  // Finally, this is the correct address range, now that only the mapping that actually corresponds
  // to the executable section is left.
  EXPECT_EQ(actual_module_update.module().address_start(), 0x101000);
  EXPECT_EQ(actual_module_update.module().address_end(), 0x103000);
  EXPECT_EQ(actual_module_update.module().build_id(), kLibtestDllBuildId);
  EXPECT_EQ(actual_module_update.module().load_bias(), kLibtestDllImageBase);
  EXPECT_EQ(actual_module_update.module().executable_segment_offset(), kLibtestDllBaseOfCode);
  EXPECT_EQ(actual_module_update.module().soname(), "");
  EXPECT_EQ(actual_module_update.module().object_file_type(),
            orbit_grpc_protos::ModuleInfo::kCoffFile);
  VerifyObjectSegmentsForLibtestDll(actual_module_update.module().object_segments());

  // 100000-101000 r--p 00000000 01:02 42    /path/to/libtest.dll
  // 101000-103000 r-xp 00000000 00:00 0
  // 103000-105000 r--p 00000000 00:00 0                             <--
  // 105000-120000 rw-p 00000000 00:00 0
  MmapPerfEvent section2_protection_mmap_data_event{
      .timestamp = 11,
      .data =
          {
              .address = 0x103000,
              .length = 0x2000,
              .page_offset = 0,
              .filename = "",
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x103000, 0x105000, 0, PROT_READ, "")).Times(1);
  PerfEvent(std::move(section2_protection_mmap_data_event)).Accept(&visitor_);

  // Map for section 3 is already writeable so no protection change event is generated.

  // 100000-101000 r--p 00000000 01:02 42    /path/to/libtest.dll
  // 101000-103000 r-xp 00000000 00:00 0
  // 103000-105000 r--p 00000000 00:00 0
  // 105000-106000 rw-p 00000000 00:00 0
  // 106000-107000 r--p 00000000 00:00 0                             <--
  // 107000-120000 rw-p 00000000 00:00 0
  MmapPerfEvent section4_protection_mmap_data_event{
      .timestamp = 12,
      .data =
          {
              .address = 0x106000,
              .length = 0x1000,
              .page_offset = 0,
              .filename = "",
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x106000, 0x107000, 0, PROT_READ, "")).Times(1);
  PerfEvent(std::move(section4_protection_mmap_data_event)).Accept(&visitor_);

  // 100000-101000 r--p 00000000 01:02 42    /path/to/libtest.dll
  // 101000-103000 r-xp 00000000 00:00 0
  // 103000-105000 r--p 00000000 00:00 0
  // 105000-106000 rw-p 00000000 00:00 0
  // 106000-11F000 r--p 00000000 00:00 0                             <--
  // 11F000-120000 rw-p 00000000 00:00 0
  MmapPerfEvent section5_protection_mmap_data_event{
      .timestamp = 13,
      .data =
          {
              .address = 0x106000,
              .length = 0x19000,
              .page_offset = 0,
              .filename = "",
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x106000, 0x11F000, 0, PROT_READ, "")).Times(1);
  PerfEvent(std::move(section5_protection_mmap_data_event)).Accept(&visitor_);

  // 100000-101000 r--p 00000000 01:02 42    /path/to/libtest.dll
  // 101000-103000 r-xp 00000000 00:00 0
  // 103000-105000 r--p 00000000 00:00 0
  // 105000-106000 rw-p 00000000 00:00 0
  // 106000-120000 r--p 00000000 00:00 0                             <--
  MmapPerfEvent section6_protection_mmap_data_event{
      .timestamp = 14,
      .data =
          {
              .address = 0x106000,
              .length = 0x1a000,
              .page_offset = 0,
              .filename = "",
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x106000, 0x120000, 0, PROT_READ, "")).Times(1);
  PerfEvent(std::move(section6_protection_mmap_data_event)).Accept(&visitor_);
}

}  // namespace orbit_linux_tracing
