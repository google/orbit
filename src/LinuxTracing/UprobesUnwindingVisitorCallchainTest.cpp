// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unwindstack/MapInfo.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "LibunwindstackMaps.h"
#include "LibunwindstackUnwinder.h"
#include "LinuxTracing/UserSpaceInstrumentationAddresses.h"
#include "MockTracerListener.h"
#include "OrbitBase/Logging.h"
#include "PerfEvent.h"
#include "PerfEventRecords.h"
#include "UprobesFunctionCallManager.h"
#include "UprobesUnwindingVisitor.h"
#include "UprobesUnwindingVisitorTestCommon.h"

using ::testing::_;
using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::Ge;
using ::testing::Invoke;
using ::testing::Lt;
using ::testing::Return;
using ::testing::SaveArg;

using orbit_grpc_protos::Callstack;

namespace orbit_linux_tracing {

namespace {

class UprobesUnwindingVisitorCallchainTest : public ::testing::Test {
 protected:
  void SetUp() override {
    EXPECT_CALL(maps_, Find(AllOf(Ge(kUprobesMapsStart), Lt(kUprobesMapsEnd))))
        .WillRepeatedly(Return(kUprobesMapInfo));

    EXPECT_CALL(maps_, Find(AllOf(Ge(kTargetMapsStart), Lt(kTargetMapsEnd))))
        .WillRepeatedly(Return(kTargetMapInfo));

    EXPECT_CALL(maps_, Find(AllOf(Ge(kNonExecutableMapsStart), Lt(kNonExecutableMapsEnd))))
        .WillRepeatedly(Return(kNonExecutableMapInfo));
  }

  static constexpr uint32_t kStackDumpSize = 128;
  MockTracerListener listener_;
  UprobesFunctionCallManager function_call_manager_;
  MockUprobesReturnAddressManager return_address_manager_{nullptr};
  MockLibunwindstackMaps maps_;
  MockLibunwindstackUnwinder unwinder_;
  MockLeafFunctionCallManager leaf_function_call_manager_{kStackDumpSize};

  static inline const std::string kUserSpaceLibraryName = "/path/to/library.so";
  static constexpr uint64_t kUserSpaceLibraryMapsStart = 0xCCCCCCCCCCCCCC00LU;
  static constexpr uint64_t kUserSpaceLibraryMapsEnd = 0xCCCCCCCCCCCCCCFFLU;
  static inline const std::shared_ptr<unwindstack::MapInfo> kUserSpaceLibraryMapInfo =
      unwindstack::MapInfo::Create(kUserSpaceLibraryMapsStart, kUserSpaceLibraryMapsEnd, 0,
                                   PROT_EXEC | PROT_READ, kUserSpaceLibraryName);

  static constexpr uint64_t kUserSpaceLibraryAddress = kUserSpaceLibraryMapsStart;
  static constexpr uint64_t kEntryTrampolineAddress = 0xAAAAAAAAAAAAAA00LU;
  static constexpr uint64_t kReturnTrampolineAddress = 0xBBBBBBBBBBBBBB00LU;

  class FakeUserSpaceInstrumentationAddresses : public UserSpaceInstrumentationAddresses {
   public:
    [[nodiscard]] bool IsInEntryTrampoline(uint64_t address) const override {
      return address == kEntryTrampolineAddress || address == kEntryTrampolineAddress + 1;
    }
    [[nodiscard]] bool IsInReturnTrampoline(uint64_t address) const override {
      return address == kReturnTrampolineAddress || address == kReturnTrampolineAddress + 1;
    }
    [[nodiscard]] std::string_view GetInjectedLibraryMapName() const override {
      return kUserSpaceLibraryName;
    }
  } user_space_instrumentation_addresses_;

  std::map<uint64_t, uint64_t> absolute_address_to_size_of_functions_to_stop_at_{};

  UprobesUnwindingVisitor visitor_{&listener_,
                                   &function_call_manager_,
                                   &return_address_manager_,
                                   &maps_,
                                   &unwinder_,
                                   &leaf_function_call_manager_,
                                   &user_space_instrumentation_addresses_,
                                   &absolute_address_to_size_of_functions_to_stop_at_};

  static constexpr uint64_t kKernelAddress = 0xFFFFFFFFFFFFFE00;

  static inline const std::string kUprobesName = "[uprobes]";
  static constexpr uint64_t kUprobesMapsStart = 0x7FFFFFFFE000;
  static constexpr uint64_t kUprobesMapsEnd = 0x7FFFFFFFE001;
  static inline const std::shared_ptr<unwindstack::MapInfo> kUprobesMapInfo =
      unwindstack::MapInfo::Create(kUprobesMapsStart, kUprobesMapsEnd, 0, PROT_EXEC | PROT_READ,
                                   kUprobesName);

  static inline const std::string kTargetName = "target";
  static constexpr uint64_t kTargetMapsStart = 100;
  static constexpr uint64_t kTargetMapsEnd = 400;
  static inline const std::shared_ptr<unwindstack::MapInfo> kTargetMapInfo =
      unwindstack::MapInfo::Create(kTargetMapsStart, kTargetMapsEnd, 0, PROT_EXEC | PROT_READ,
                                   kTargetName);

  static constexpr uint64_t kTargetAddress1 = 100;
  static constexpr uint64_t kTargetAddress2 = 200;
  static constexpr uint64_t kTargetAddress3 = 300;

  static constexpr uint64_t kNonExecutableMapsStart = 500;
  static constexpr uint64_t kNonExecutableMapsEnd = 600;
  static inline const std::string kNonExecutableName = "data";

  static inline const std::shared_ptr<unwindstack::MapInfo> kNonExecutableMapInfo =
      unwindstack::MapInfo::Create(kNonExecutableMapsStart, kNonExecutableMapsEnd, 0,
                                   PROT_EXEC | PROT_READ, kNonExecutableName);
};

CallchainSamplePerfEvent BuildFakeCallchainSamplePerfEvent(absl::Span<const uint64_t> callchain) {
  constexpr uint64_t kTotalNumOfRegisters =
      sizeof(perf_event_sample_regs_user_all) / sizeof(uint64_t);

  constexpr uint64_t kStackSize = 13;
  CallchainSamplePerfEvent event{
      .timestamp = 15,
      .data =
          {
              .pid = 10,
              .tid = 11,
              .regs = std::make_unique<uint64_t[]>(kTotalNumOfRegisters),
              .data = std::make_unique<uint8_t[]>(kStackSize),
          },
  };
  event.data.SetIps(callchain);
  return event;
}

}  // namespace

TEST_F(UprobesUnwindingVisitorCallchainTest,
       VisitValidCallchainSampleWithoutUprobesSendsCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kTargetAddress1,
      // Increment by one as the return address is the next address.
      kTargetAddress2 + 1,
      kTargetAddress3 + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(1).WillOnce(Return(true));
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction)
      .Times(1)
      .WillOnce(Return(Callstack::kComplete));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress2, kTargetAddress3));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorCallchainTest, VisitSingleFrameCallchainSampleDoesNothing) {
  std::vector<uint64_t> callchain{kKernelAddress};

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(0);
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction).Times(0);

  EXPECT_CALL(listener_, OnCallstackSample).Times(0);

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorCallchainTest,
       VisitCallchainSampleInsideUprobeCodeSendsInUprobesCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kUprobesMapsStart,
      // Increment by one as the return address is the next address.
      kTargetAddress2 + 1,
      kTargetAddress3 + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find(_)).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(maps_, Find(kUprobesMapsStart)).WillRepeatedly(Return(kUprobesMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(0);
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction).Times(0);

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kUprobesMapsStart, kTargetAddress2, kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kInUprobes);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 1);
}

TEST_F(
    UprobesUnwindingVisitorCallchainTest,
    VisitCallchainSampleInsideUserSpaceInstrumentationTrampolineSendsInUserSpaceInstrumentationCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kEntryTrampolineAddress,
      // Increment by one as the return address is the next address.
      kTargetAddress2 + 1,
      kTargetAddress3 + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find(_)).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(maps_, Find(kEntryTrampolineAddress)).WillRepeatedly(Return(nullptr));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(0);
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction).Times(0);

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kEntryTrampolineAddress, kTargetAddress2, kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kInUserSpaceInstrumentation);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(
    UprobesUnwindingVisitorCallchainTest,
    VisitCallchainSampleInsideUserSpaceInstrumentationLibrarySendsInUserSpaceInstrumentationCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kTargetAddress1,
      // Increment by one as the return address is the next address.
      kUserSpaceLibraryAddress + 1,
      kTargetAddress3 + 1,
      kEntryTrampolineAddress + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find(_)).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(maps_, Find(AllOf(Ge(kUserSpaceLibraryMapsStart), Lt(kUserSpaceLibraryMapsEnd))))
      .WillRepeatedly(Return(kUserSpaceLibraryMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(0);
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction).Times(0);

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  // While this is a Callstack::kInUserSpaceInstrumentation, the innermost frame we used is still
  // one of the "regular" frames in the target, i.e., at kTargetAddress1.
  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kUserSpaceLibraryAddress, kTargetAddress3,
                          kEntryTrampolineAddress));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kInUserSpaceInstrumentation);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(
    UprobesUnwindingVisitorCallchainTest,
    VisitCallchainSampleInsideUserSpaceInstrumentationLibraryAfterLeafFunctionPatchingSendsInUserSpaceInstrumentationCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kTargetAddress1,
      // Increment by one as the return address is the next address.
      // `kUserSpaceLibraryAddress + 1` is the missing frame.
      kTargetAddress3 + 1,
      kEntryTrampolineAddress + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find(_)).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(maps_, Find(AllOf(Ge(kUserSpaceLibraryMapsStart), Lt(kUserSpaceLibraryMapsEnd))))
      .WillRepeatedly(Return(kUserSpaceLibraryMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(0);
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction)
      .Times(1)
      .WillOnce([](const CallchainSamplePerfEventData* event_data,
                   LibunwindstackMaps* /*current_maps*/, LibunwindstackUnwinder* /*unwinder*/) {
        event_data->SetIps({
            kKernelAddress,
            kTargetAddress1,
            kUserSpaceLibraryAddress + 1,  // This was the missing frame.
            kTargetAddress3 + 1,
            kEntryTrampolineAddress + 1,
        });
        return Callstack::kComplete;
      });

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  // While this is a Callstack::kInUserSpaceInstrumentation, the innermost frame we used is still
  // one of the "regular" frames in the target, i.e., at kTargetAddress1.
  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kUserSpaceLibraryAddress, kTargetAddress3,
                          kEntryTrampolineAddress));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kInUserSpaceInstrumentation);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorCallchainTest, VisitPatchableCallchainSampleSendsCompleteCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kTargetAddress1,
      // Increment by one as the return address is the next address.
      kUprobesMapsStart + 1,
      kTargetAddress3 + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(kTargetMapInfo));
  auto fake_patch_callchain = [](pid_t /*tid*/, uint64_t* callchain, uint64_t callchain_size,
                                 orbit_linux_tracing::LibunwindstackMaps*) -> bool {
    ORBIT_CHECK(callchain != nullptr);
    ORBIT_CHECK(callchain_size == 4);
    callchain[2] = kTargetAddress2 + 1;
    return true;
  };
  EXPECT_CALL(return_address_manager_, PatchCallchain)
      .Times(1)
      .WillOnce(Invoke(fake_patch_callchain));

  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction)
      .Times(1)
      .WillOnce(Return(Callstack::kComplete));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress2, kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kComplete);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorCallchainTest,
       VisitUnpatchableCallchainSampleSendsPatchingFailedCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kTargetAddress1,
      // Increment by one as the return address is the next address.
      kUprobesMapsStart + 1,
      kTargetAddress3 + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction)
      .Times(1)
      .WillOnce(Return(Callstack::kComplete));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(1).WillOnce(Return(false));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kUprobesMapsStart, kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kCallstackPatchingFailed);

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorCallchainTest,
       VisitLeafCallOptimizedCallchainSampleWithoutUprobesSendsCompleteCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kTargetAddress1,
      // Increment by one as the return address is the next address.
      kTargetAddress3 + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(1).WillRepeatedly(Return(true));

  auto fake_patch_caller_of_leaf_function =
      [](const CallchainSamplePerfEventData* event_data, LibunwindstackMaps* /*maps*/,
         orbit_linux_tracing::LibunwindstackUnwinder*) -> Callstack::CallstackType {
    ORBIT_CHECK(event_data != nullptr);
    std::vector<uint64_t> patched_callchain;
    EXPECT_THAT(event_data->CopyOfIpsAsVector(),
                ElementsAre(kKernelAddress, kTargetAddress1, kTargetAddress3 + 1));
    patched_callchain.push_back(kKernelAddress);
    patched_callchain.push_back(kTargetAddress1);
    // Patch in the missing frame:
    patched_callchain.push_back(kTargetAddress2 + 1);
    patched_callchain.push_back(kTargetAddress3 + 1);
    event_data->SetIps(patched_callchain);
    return Callstack::kComplete;
  };
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction)
      .Times(1)
      .WillOnce(Invoke(fake_patch_caller_of_leaf_function));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress2, kTargetAddress3));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(
    UprobesUnwindingVisitorCallchainTest,
    VisitLeafCallOptimizedCallchainSampleWherePatchingLeafFunctionCallerFailsSendsFramePointerUnwindingErrorCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kTargetAddress1,
      // Increment by one as the return address is the next address.
      kTargetAddress3 + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(0);

  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction)
      .Times(1)
      .WillOnce(Return(Callstack::kFramePointerUnwindingError));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kFramePointerUnwindingError);

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

}  // namespace orbit_linux_tracing
