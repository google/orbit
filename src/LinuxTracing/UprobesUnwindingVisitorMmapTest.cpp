// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/mman.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "GrpcProtos/capture.pb.h"
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
                              const std::string& name) {
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
  orbit_grpc_protos::ModuleUpdateEvent actual_module_update;
  EXPECT_CALL(listener_, OnModuleUpdate).Times(1).WillOnce(SaveArg<0>(&actual_module_update));
  PerfEvent(std::move(file_mmap_event)).Accept(&visitor_);
  EXPECT_EQ(actual_module_update.pid(), kPid);
  EXPECT_EQ(actual_module_update.timestamp_ns(), 3);
  EXPECT_EQ(actual_module_update.module().name(), "target_fp");
  EXPECT_EQ(actual_module_update.module().file_path(), test_binary_path);
  EXPECT_EQ(actual_module_update.module().file_size(), 27824);
  EXPECT_EQ(actual_module_update.module().address_start(), 0x55bf53c22000);
  EXPECT_EQ(actual_module_update.module().address_end(), 0x55bf53c24000);
  EXPECT_EQ(actual_module_update.module().build_id(), "d7e2447f79faa88528dd0d130ac7cc5f168ca090");
  EXPECT_EQ(actual_module_update.module().load_bias(), 0);
  EXPECT_EQ(actual_module_update.module().executable_segment_offset(), 0x1000);
  EXPECT_EQ(actual_module_update.module().soname(), "");
  EXPECT_EQ(actual_module_update.module().object_file_type(),
            orbit_grpc_protos::ModuleInfo::kElfFile);

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
  PerfEvent(std::move(bad_file_mmap_event)).Accept(&visitor_);
}

TEST_F(UprobesUnwindingVisitorMmapTest,
       VisitMmapPerfEventSendsModuleUpdatesFromPeCoffTextSectionInAnonExecMap) {
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
  EXPECT_EQ(actual_module_update.module().file_size(), 96441);
  EXPECT_EQ(actual_module_update.module().address_start(), 0x101000);
  EXPECT_EQ(actual_module_update.module().address_end(), 0x103000);
  EXPECT_EQ(actual_module_update.module().build_id(), "");
  EXPECT_EQ(actual_module_update.module().load_bias(), 0x62640000);
  EXPECT_EQ(actual_module_update.module().executable_segment_offset(), 0x1000);
  EXPECT_EQ(actual_module_update.module().soname(), "");
  EXPECT_EQ(actual_module_update.module().object_file_type(),
            orbit_grpc_protos::ModuleInfo::kCoffFile);
}

}  // namespace orbit_linux_tracing
