// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "ClientData/CallstackEvent.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/CallstackType.h"
#include "ClientData/CaptureData.h"
#include "ClientData/LinuxAddressInfo.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "ClientModel/SamplingDataPostProcessor.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Sort.h"
#include "OrbitBase/ThreadConstants.h"

using orbit_client_data::CallstackCount;
using orbit_client_data::CallstackEvent;
using orbit_client_data::CallstackInfo;
using orbit_client_data::CallstackType;
using orbit_client_data::CaptureData;
using orbit_client_data::LinuxAddressInfo;
using orbit_client_data::ModuleManager;
using orbit_client_data::PostProcessedSamplingData;
using orbit_client_data::SampledFunction;
using orbit_client_data::SortedCallstackReport;
using orbit_client_data::ThreadSampleData;

using orbit_grpc_protos::CaptureStarted;

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Pointwise;
using ::testing::UnorderedElementsAre;

namespace orbit_client_model {

namespace {

SampledFunction MakeSampledFunction(std::string name, std::string module_path, uint32_t exclusive,
                                    float exclusive_percent, uint32_t inclusive,
                                    float inclusive_percent, uint32_t unwind_errors,
                                    float unwind_errors_percent, uint64_t absolute_address) {
  SampledFunction sampled_function;
  sampled_function.name = std::move(name);
  sampled_function.module_path = std::move(module_path);
  sampled_function.exclusive = exclusive;
  sampled_function.exclusive_percent = exclusive_percent;
  sampled_function.inclusive = inclusive;
  sampled_function.inclusive_percent = inclusive_percent;
  sampled_function.unwind_errors = unwind_errors;
  sampled_function.unwind_errors_percent = unwind_errors_percent;
  sampled_function.absolute_address = absolute_address;
  sampled_function.function = nullptr;
  return sampled_function;
}

bool SampledFunctionsAreEqual(const SampledFunction& lhs, const SampledFunction& rhs) {
  return lhs.name == rhs.name && lhs.module_path == rhs.module_path &&
         lhs.exclusive == rhs.exclusive && lhs.exclusive_percent == rhs.exclusive_percent &&
         lhs.inclusive == rhs.inclusive && lhs.inclusive_percent == rhs.inclusive_percent &&
         lhs.unwind_errors == rhs.unwind_errors &&
         lhs.unwind_errors_percent == rhs.unwind_errors_percent &&
         lhs.absolute_address == rhs.absolute_address && lhs.function == rhs.function;
}

MATCHER_P(SampledFunctionEq, that, "") {
  const SampledFunction& lhs = arg;
  const SampledFunction& rhs = that;
  return SampledFunctionsAreEqual(lhs, rhs);
}

bool CallstackIdToCallstackEventPairsAreEqual(
    const std::pair<uint64_t, std::vector<CallstackEvent>>& lhs,
    const std::pair<uint64_t, std::vector<CallstackEvent>>& rhs) {
  auto callstack_events_projector = [](const CallstackEvent& event) {
    return std::make_tuple(event.timestamp_ns(), event.callstack_id(), event.thread_id());
  };
  auto callstack_events_equal = [&callstack_events_projector](const CallstackEvent& lhs,
                                                              const CallstackEvent& rhs) {
    return callstack_events_projector(lhs) == callstack_events_projector(rhs);
  };

  std::vector<CallstackEvent> lhs_events = lhs.second;
  orbit_base::sort(lhs_events.begin(), lhs_events.end(), callstack_events_projector);
  std::vector<CallstackEvent> rhs_events = rhs.second;
  orbit_base::sort(rhs_events.begin(), rhs_events.end(), callstack_events_projector);
  return lhs.first == rhs.first &&
         std::equal(lhs_events.begin(), lhs_events.end(), rhs_events.begin(), rhs_events.end(),
                    callstack_events_equal);
};

MATCHER_P(CallstackIdToCallstackEventsEq, that, "") {
  const std::pair<uint64_t, std::vector<CallstackEvent>>& lhs = arg;
  const std::pair<uint64_t, std::vector<CallstackEvent>>& rhs = that;
  return CallstackIdToCallstackEventPairsAreEqual(lhs, rhs);
}

SortedCallstackReport MakeSortedCallstackReport(
    absl::Span<const std::pair<int, uint64_t>> counts_and_callstack_ids) {
  SortedCallstackReport report{};
  for (const auto& [count, callstack_id] : counts_and_callstack_ids) {
    report.total_callstack_count += count;
    CallstackCount callstack_count;
    callstack_count.count = count;
    callstack_count.callstack_id = callstack_id;
    report.callstack_counts.push_back(callstack_count);
  }
  return report;
}

MATCHER_P(SortedCallstackReportEq, that, "") {
  const SortedCallstackReport& lhs = arg;
  const SortedCallstackReport& rhs = that;

  // `SortedCallstackReport::callstacks_counts` is sorted only by `CallstackCount::count` (in
  // descending order), hence the order is not unique. Sort again considering
  // `CallstackCount::callstack_id`, too  (also descending, it doesn't matter which way), to
  // facilitate the comparison of `SortedCallstackReport`s for equality.
  auto callstack_count_projector = [](const CallstackCount& callstack_count) {
    return std::make_pair(callstack_count.count, callstack_count.callstack_id);
  };

  std::vector<CallstackCount> lhs_resorted_callstacks_counts = lhs.callstack_counts;
  orbit_base::sort(lhs_resorted_callstacks_counts.begin(), lhs_resorted_callstacks_counts.end(),
                   callstack_count_projector, std::greater<>{});

  std::vector<CallstackCount> rhs_resorted_callstacks_counts = rhs.callstack_counts;
  orbit_base::sort(rhs_resorted_callstacks_counts.begin(), rhs_resorted_callstacks_counts.end(),
                   callstack_count_projector, std::greater<>{});

  auto callstack_count_equal = [&callstack_count_projector](const CallstackCount& lhs,
                                                            const CallstackCount& rhs) {
    return callstack_count_projector(lhs) == callstack_count_projector(rhs);
  };

  return lhs.total_callstack_count == rhs.total_callstack_count &&
         std::equal(lhs_resorted_callstacks_counts.begin(), lhs_resorted_callstacks_counts.end(),
                    rhs_resorted_callstacks_counts.begin(), rhs_resorted_callstacks_counts.end(),
                    callstack_count_equal);
}

class SamplingDataPostProcessorTest : public ::testing::Test {
 protected:
  void SetUp() override {}

  void TearDown() override {}

  CaptureData capture_data_{CaptureStarted{}, std::filesystem::path{},
                            absl::flat_hash_set<uint64_t>{}, CaptureData::DataSource::kLiveCapture};

  void AddCallstackInfo(uint64_t callstack_id, absl::Span<const uint64_t> callstack_frames,
                        CallstackType callstack_type) {
    CallstackInfo callstack_info{{callstack_frames.begin(), callstack_frames.end()},
                                 callstack_type};

    capture_data_.AddUniqueCallstack(callstack_id, std::move(callstack_info));
  }

  void AddCallstackEvent(uint64_t callstack_id, uint32_t thread_id) {
    current_callstack_timestamp_ns_ += 100;
    CallstackEvent callstack_event{current_callstack_timestamp_ns_, callstack_id, thread_id};
    capture_data_.AddCallstackEvent(callstack_event);
  }

  void AddAddressInfo(std::string module_path, std::string function_name, uint64_t absolute_address,
                      uint64_t offset_in_function) {
    LinuxAddressInfo linux_address_info{absolute_address, offset_in_function,
                                        std::move(module_path), std::move(function_name)};
    capture_data_.InsertAddressInfo(std::move(linux_address_info));
  }

  static const inline std::string kModulePath = "/path/to/module";

  static const inline std::string kFunction1Name = "function1";
  static constexpr uint64_t kFunction1StartAbsoluteAddress = 0x10;
  static constexpr uint64_t kFunction1Instruction1AbsoluteAddress = 0x11;
  static constexpr uint64_t kFunction1Instruction1OffsetInFunction =
      kFunction1Instruction1AbsoluteAddress - kFunction1StartAbsoluteAddress;

  static const inline std::string kFunction2Name = "function2";
  static constexpr uint64_t kFunction2StartAbsoluteAddress = 0x20;
  static constexpr uint64_t kFunction2Instruction1AbsoluteAddress = 0x21;
  static constexpr uint64_t kFunction2Instruction1OffsetInFunction =
      kFunction2Instruction1AbsoluteAddress - kFunction2StartAbsoluteAddress;

  static const inline std::string kFunction3Name = "function3";
  static constexpr uint64_t kFunction3StartAbsoluteAddress = 0x30;
  static constexpr uint64_t kFunction3Instruction1AbsoluteAddress = 0x31;
  static constexpr uint64_t kFunction3Instruction1OffsetInFunction =
      kFunction3Instruction1AbsoluteAddress - kFunction3StartAbsoluteAddress;
  static constexpr uint64_t kFunction3Instruction2AbsoluteAddress = 0x32;
  static constexpr uint64_t kFunction3Instruction2OffsetInFunction =
      kFunction3Instruction2AbsoluteAddress - kFunction3StartAbsoluteAddress;

  static const inline std::string kFunction4Name = "function4";
  static constexpr uint64_t kFunction4StartAbsoluteAddress = 0x40;
  static constexpr uint64_t kFunction4Instruction1AbsoluteAddress = 0x41;
  static constexpr uint64_t kFunction4Instruction1OffsetInFunction =
      kFunction4Instruction1AbsoluteAddress - kFunction4StartAbsoluteAddress;

  void AddAllAddressInfos() {
    AddAddressInfo(kModulePath, kFunction1Name, kFunction1Instruction1AbsoluteAddress,
                   kFunction1Instruction1OffsetInFunction);

    AddAddressInfo(kModulePath, kFunction2Name, kFunction2Instruction1AbsoluteAddress,
                   kFunction2Instruction1OffsetInFunction);

    AddAddressInfo(kModulePath, kFunction3Name, kFunction3Instruction1AbsoluteAddress,
                   kFunction3Instruction1OffsetInFunction);
    AddAddressInfo(kModulePath, kFunction3Name, kFunction3Instruction2AbsoluteAddress,
                   kFunction3Instruction2OffsetInFunction);

    AddAddressInfo(kModulePath, kFunction4Name, kFunction4Instruction1AbsoluteAddress,
                   kFunction4Instruction1OffsetInFunction);
  }

  // See AddCallstackEventsAllInThreadId1 for the meaning of the letters next to the frames.
  static constexpr uint64_t kCallstack1Id = 1;
  static const inline std::vector<uint64_t> kCallstack1Frames{
      kFunction3Instruction1AbsoluteAddress,  // C
      kFunction2Instruction1AbsoluteAddress,  // B
      kFunction1Instruction1AbsoluteAddress,  // A
  };
  static const inline std::vector<uint64_t> kCallstack1ResolvedFrames{
      kFunction3StartAbsoluteAddress,
      kFunction2StartAbsoluteAddress,
      kFunction1StartAbsoluteAddress,
  };

  static constexpr uint64_t kCallstack2Id = 2;
  static const inline std::vector<uint64_t> kCallstack2Frames{
      kFunction4Instruction1AbsoluteAddress,  // D
      kFunction3Instruction1AbsoluteAddress,  // C
      kFunction2Instruction1AbsoluteAddress,  // B
      kFunction1Instruction1AbsoluteAddress,  // A
  };
  static const inline std::vector<uint64_t> kCallstack2ResolvedFrames{
      kFunction4StartAbsoluteAddress,
      kFunction3StartAbsoluteAddress,
      kFunction2StartAbsoluteAddress,
      kFunction1StartAbsoluteAddress,
  };

  static constexpr uint64_t kCallstack3Id = 3;
  static const inline std::vector<uint64_t> kCallstack3Frames{
      kFunction3Instruction2AbsoluteAddress,  // C'
      kFunction3Instruction1AbsoluteAddress,  // C
      kFunction1Instruction1AbsoluteAddress,  // A
  };
  static const inline std::vector<uint64_t> kCallstack3ResolvedFrames{
      kFunction3StartAbsoluteAddress,
      kFunction3StartAbsoluteAddress,
      kFunction1StartAbsoluteAddress,
  };

  static constexpr uint64_t kCallstack4Id = 4;
  static const inline std::vector<uint64_t> kCallstack4Frames{
      kFunction3Instruction1AbsoluteAddress,  // C
      kFunction3Instruction1AbsoluteAddress,  // C
      kFunction1Instruction1AbsoluteAddress,  // A
  };
  static const inline std::vector<uint64_t> kCallstack4ResolvedFrames{
      kFunction3StartAbsoluteAddress,
      kFunction3StartAbsoluteAddress,
      kFunction1StartAbsoluteAddress,
  };

  void AddAllCallstackInfos(CallstackType callstack_type) {
    AddCallstackInfo(kCallstack1Id, kCallstack1Frames, callstack_type);
    AddCallstackInfo(kCallstack2Id, kCallstack2Frames, callstack_type);
    AddCallstackInfo(kCallstack3Id, kCallstack3Frames, callstack_type);
    AddCallstackInfo(kCallstack4Id, kCallstack4Frames, callstack_type);
  }

  void AddAllCallstackInfosWithMixedCallstackTypes() {
    AddCallstackInfo(kCallstack1Id, kCallstack1Frames, CallstackType::kDwarfUnwindingError);
    AddCallstackInfo(kCallstack2Id, kCallstack2Frames, CallstackType::kComplete);
    AddCallstackInfo(kCallstack3Id, kCallstack3Frames, CallstackType::kComplete);
    AddCallstackInfo(kCallstack4Id, kCallstack4Frames,
                     CallstackType::kFilteredByMajorityOutermostFrame);
  }

  static constexpr int32_t kThreadId1 = 42;
  static constexpr int32_t kThreadId2 = 43;
  static constexpr int32_t kThreadIdNotSampled = 99;

  void AddCallstackEventsAllInThreadId1() {
    // Let:
    // A  = kFunction1Instruction1AbsoluteAddress,
    // B  = kFunction2Instruction1AbsoluteAddress,
    // C  = kFunction3Instruction1AbsoluteAddress,
    // C' = kFunction3Instruction2AbsoluteAddress,
    // D  = kFunction4Instruction1AbsoluteAddress.
    // These are the CallstackEvents that are added, innermost frame at the top. Note that:
    //   - the first and second CallstackEvents have the same Callstack;
    //   - the last CallstackEvent has two identical frames;
    //   - the number under each CallstackEvent is the callstack id.
    //           D
    // C    C    C    C'   C
    // B    B    B    C    C
    // A    A    A    A    A
    // ---------------------
    // 1    1    2    3    4
    //
    // Note that when the CallstackInfos are added with AddAllCallstackInfosWithMixedCallstackTypes,
    // for statistics and selection the added CallstackEvents are effectively the following. The 'E'
    // indicates which CallstackEvents refer to a non-kComplete CallstackInfo.
    //           D
    //           C    C'
    //           B    C
    // C    C    A    A    C
    // ---------------------
    // 1    1    2    3    4
    // E    E              E
    AddCallstackEvent(kCallstack1Id, kThreadId1);
    AddCallstackEvent(kCallstack1Id,
                      kThreadId1);  // Intentionally two CallstackEvents with the same CallstackInfo
    AddCallstackEvent(kCallstack2Id, kThreadId1);
    AddCallstackEvent(kCallstack3Id, kThreadId1);
    AddCallstackEvent(kCallstack4Id, kThreadId1);
  }

  void AddCallstackEventsInThreadId1And2() {
    // Like above, but with CallstackEvents split between kThreadId1 and kThreadId2 as follows:
    // kThreadId1 |     kThreadId2
    //      D
    // C    C     |     C    C'   C
    // B    B     |     B    C    C
    // A    A     |     A    A    A
    // ----------------------------
    // 1    2     |     1    3    4
    //
    // When the CallstackInfos are added with AddAllCallstackInfosWithMixedCallstackTypes:
    // kThreadId1 |     kThreadId2
    //      D
    //      C     |          C'
    //      B     |          C
    // C    A     |     C    A    C
    // ----------------------------
    // 1    2     |     1    3    4
    // E          |     E         E
    AddCallstackEvent(kCallstack1Id, kThreadId1);
    AddCallstackEvent(kCallstack2Id, kThreadId1);

    AddCallstackEvent(kCallstack1Id, kThreadId2);
    AddCallstackEvent(kCallstack3Id, kThreadId2);
    AddCallstackEvent(kCallstack4Id, kThreadId2);
  }

  void CreatePostProcessedSamplingDataWithoutSummary() {
    orbit_client_data::ModuleManager module_manager{};
    ppsd_ = CreatePostProcessedSamplingData(capture_data_.GetCallstackData(), capture_data_,
                                            module_manager,
                                            /*generate_summary=*/false);
  }

  void CreatePostProcessedSamplingDataWithSummary() {
    orbit_client_data::ModuleManager module_manager{};
    ppsd_ = CreatePostProcessedSamplingData(capture_data_.GetCallstackData(), capture_data_,
                                            module_manager,
                                            /*generate_summary=*/true);
  }

  PostProcessedSamplingData ppsd_;

  void VerifyNoCallstackInfos() {
    EXPECT_DEATH((void)ppsd_.GetResolvedCallstack(kCallstack1Id), "");
    EXPECT_DEATH((void)ppsd_.GetResolvedCallstack(kCallstack2Id), "");
    EXPECT_DEATH((void)ppsd_.GetResolvedCallstack(kCallstack3Id), "");
    EXPECT_DEATH((void)ppsd_.GetResolvedCallstack(kCallstack4Id), "");
  }

  void VerifyAllCallstackInfos(CallstackType expected_callstack_type) {
    {
      const CallstackInfo& resolved_callstack_1 = ppsd_.GetResolvedCallstack(kCallstack1Id);
      EXPECT_THAT(resolved_callstack_1.frames(), Pointwise(Eq(), kCallstack1ResolvedFrames));
      EXPECT_EQ(resolved_callstack_1.type(), expected_callstack_type);
    }

    {
      const CallstackInfo& resolved_callstack_2 = ppsd_.GetResolvedCallstack(kCallstack2Id);
      EXPECT_THAT(resolved_callstack_2.frames(), Pointwise(Eq(), kCallstack2ResolvedFrames));
      EXPECT_EQ(resolved_callstack_2.type(), expected_callstack_type);
    }

    {
      const CallstackInfo& resolved_callstack_3 = ppsd_.GetResolvedCallstack(kCallstack3Id);
      EXPECT_THAT(resolved_callstack_3.frames(), Pointwise(Eq(), kCallstack3ResolvedFrames));
      EXPECT_EQ(resolved_callstack_3.type(), expected_callstack_type);
    }

    {
      const CallstackInfo& resolved_callstack_4 = ppsd_.GetResolvedCallstack(kCallstack4Id);
      EXPECT_THAT(resolved_callstack_4.frames(), Pointwise(Eq(), kCallstack4ResolvedFrames));
      EXPECT_EQ(resolved_callstack_4.type(), expected_callstack_type);
    }
  }

  void VerifyAllCallstackInfosWithMixedCallstackTypes() {
    {
      const CallstackInfo& resolved_callstack_1 = ppsd_.GetResolvedCallstack(kCallstack1Id);
      EXPECT_THAT(resolved_callstack_1.frames(), Pointwise(Eq(), kCallstack1ResolvedFrames));
      EXPECT_EQ(resolved_callstack_1.type(), CallstackType::kDwarfUnwindingError);
    }

    {
      const CallstackInfo& resolved_callstack_2 = ppsd_.GetResolvedCallstack(kCallstack2Id);
      EXPECT_THAT(resolved_callstack_2.frames(), Pointwise(Eq(), kCallstack2ResolvedFrames));
      EXPECT_EQ(resolved_callstack_2.type(), CallstackType::kComplete);
    }

    {
      const CallstackInfo& resolved_callstack_3 = ppsd_.GetResolvedCallstack(kCallstack3Id);
      EXPECT_THAT(resolved_callstack_3.frames(), Pointwise(Eq(), kCallstack3ResolvedFrames));
      EXPECT_EQ(resolved_callstack_3.type(), CallstackType::kComplete);
    }

    {
      const CallstackInfo& resolved_callstack_4 = ppsd_.GetResolvedCallstack(kCallstack4Id);
      EXPECT_THAT(resolved_callstack_4.frames(), Pointwise(Eq(), kCallstack4ResolvedFrames));
      EXPECT_EQ(resolved_callstack_4.type(), CallstackType::kFilteredByMajorityOutermostFrame);
    }
  }

  void VerifyAllCallstackInfosWithoutAddressInfos(CallstackType expected_callstack_type) {
    {
      const CallstackInfo& resolved_callstack_1 = ppsd_.GetResolvedCallstack(kCallstack1Id);
      EXPECT_THAT(resolved_callstack_1.frames(), Pointwise(Eq(), kCallstack1Frames));
      EXPECT_EQ(resolved_callstack_1.type(), expected_callstack_type);
    }

    {
      const CallstackInfo& resolved_callstack_2 = ppsd_.GetResolvedCallstack(kCallstack2Id);
      EXPECT_THAT(resolved_callstack_2.frames(), Pointwise(Eq(), kCallstack2Frames));
      EXPECT_EQ(resolved_callstack_2.type(), expected_callstack_type);
    }

    {
      const CallstackInfo& resolved_callstack_3 = ppsd_.GetResolvedCallstack(kCallstack3Id);
      EXPECT_THAT(resolved_callstack_3.frames(), Pointwise(Eq(), kCallstack3Frames));
      EXPECT_EQ(resolved_callstack_3.type(), expected_callstack_type);
    }

    {
      const CallstackInfo& resolved_callstack_4 = ppsd_.GetResolvedCallstack(kCallstack4Id);
      EXPECT_THAT(resolved_callstack_4.frames(), Pointwise(Eq(), kCallstack4Frames));
      EXPECT_EQ(resolved_callstack_4.type(), expected_callstack_type);
    }
  }

  static void VerifyThreadSampleDataForCallstackEventsAllInTheSameThread(
      const ThreadSampleData& actual_thread_sample_data, uint32_t expected_thread_id) {
    EXPECT_EQ(actual_thread_sample_data.thread_id, expected_thread_id);
    EXPECT_EQ(actual_thread_sample_data.samples_count, 5);
    EXPECT_EQ(actual_thread_sample_data.unwinding_errors_count, 0);
    EXPECT_THAT(
        actual_thread_sample_data.sampled_callstack_id_to_events,
        UnorderedElementsAre(
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack1Id, std::vector<CallstackEvent>{{100, kCallstack1Id, kThreadId1},
                                                           {200, kCallstack1Id, kThreadId1}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack2Id, std::vector<CallstackEvent>{{300, kCallstack2Id, kThreadId1}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack3Id, std::vector<CallstackEvent>{{400, kCallstack3Id, kThreadId1}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack4Id, std::vector<CallstackEvent>{{500, kCallstack4Id, kThreadId1}}))));
    EXPECT_THAT(actual_thread_sample_data.sampled_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1Instruction1AbsoluteAddress, 5),
                                     std::make_pair(kFunction2Instruction1AbsoluteAddress, 3),
                                     std::make_pair(kFunction3Instruction1AbsoluteAddress, 5),
                                     std::make_pair(kFunction3Instruction2AbsoluteAddress, 1),
                                     std::make_pair(kFunction4Instruction1AbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1StartAbsoluteAddress, 5),
                                     std::make_pair(kFunction2StartAbsoluteAddress, 3),
                                     std::make_pair(kFunction3StartAbsoluteAddress, 5),
                                     std::make_pair(kFunction4StartAbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_exclusive_count,
                UnorderedElementsAre(std::make_pair(kFunction3StartAbsoluteAddress, 4),
                                     std::make_pair(kFunction4StartAbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.sorted_count_to_resolved_address,
                UnorderedElementsAre(std::make_pair(5, kFunction1StartAbsoluteAddress),
                                     std::make_pair(3, kFunction2StartAbsoluteAddress),
                                     std::make_pair(5, kFunction3StartAbsoluteAddress),
                                     std::make_pair(1, kFunction4StartAbsoluteAddress)));
    EXPECT_THAT(
        actual_thread_sample_data.sampled_functions,
        UnorderedElementsAre(
            SampledFunctionEq(MakeSampledFunction(kFunction1Name, kModulePath, 0, 0.0f, 5, 100.0f,
                                                  0, 0.0f, kFunction1StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction2Name, kModulePath, 0, 0.0f, 3, 60.0f, 0,
                                                  0.0f, kFunction2StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction3Name, kModulePath, 4, 80.0f, 5, 100.0f,
                                                  0, 0.0f, kFunction3StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction4Name, kModulePath, 1, 20.0f, 1, 20.0f,
                                                  0, 0.0f, kFunction4StartAbsoluteAddress))));
  }

  static void VerifyThreadSampleDataForCallstackEventsAllInTheSameThreadWithMixedCallstackTypes(
      const ThreadSampleData& actual_thread_sample_data, uint32_t expected_thread_id) {
    EXPECT_EQ(actual_thread_sample_data.thread_id, expected_thread_id);
    EXPECT_EQ(actual_thread_sample_data.samples_count, 5);
    EXPECT_EQ(actual_thread_sample_data.unwinding_errors_count, 3);
    EXPECT_THAT(
        actual_thread_sample_data.sampled_callstack_id_to_events,
        UnorderedElementsAre(
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack1Id,
                std::vector<CallstackEvent>{CallstackEvent{100, kCallstack1Id, kThreadId1},
                                            CallstackEvent{200, kCallstack1Id, kThreadId1}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack2Id, std::vector<CallstackEvent>{{300, kCallstack2Id, kThreadId1}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack3Id, std::vector<CallstackEvent>{{400, kCallstack3Id, kThreadId1}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack4Id,
                std::vector<CallstackEvent>{CallstackEvent{500, kCallstack4Id, kThreadId1}}))));
    EXPECT_THAT(actual_thread_sample_data.sampled_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1Instruction1AbsoluteAddress, 2),
                                     std::make_pair(kFunction2Instruction1AbsoluteAddress, 1),
                                     std::make_pair(kFunction3Instruction1AbsoluteAddress, 5),
                                     std::make_pair(kFunction3Instruction2AbsoluteAddress, 1),
                                     std::make_pair(kFunction4Instruction1AbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1StartAbsoluteAddress, 2),
                                     std::make_pair(kFunction2StartAbsoluteAddress, 1),
                                     std::make_pair(kFunction3StartAbsoluteAddress, 5),
                                     std::make_pair(kFunction4StartAbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_exclusive_count,
                UnorderedElementsAre(std::make_pair(kFunction3StartAbsoluteAddress, 4),
                                     std::make_pair(kFunction4StartAbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.sorted_count_to_resolved_address,
                UnorderedElementsAre(std::make_pair(2, kFunction1StartAbsoluteAddress),
                                     std::make_pair(1, kFunction2StartAbsoluteAddress),
                                     std::make_pair(5, kFunction3StartAbsoluteAddress),
                                     std::make_pair(1, kFunction4StartAbsoluteAddress)));
    EXPECT_THAT(
        actual_thread_sample_data.sampled_functions,
        UnorderedElementsAre(
            SampledFunctionEq(MakeSampledFunction(kFunction1Name, kModulePath, 0, 0.0f, 2, 40.0f, 0,
                                                  0.0f, kFunction1StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction2Name, kModulePath, 0, 0.0f, 1, 20.0f, 0,
                                                  0.0f, kFunction2StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction3Name, kModulePath, 4, 80.0f, 5, 100.0f,
                                                  3, 60.0f, kFunction3StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction4Name, kModulePath, 1, 20.0f, 1, 20.0f,
                                                  0, 0.0f, kFunction4StartAbsoluteAddress))));
  }

  static void
  VerifyThreadSampleDataForCallstackEventsAllInTheSameThreadWithOnlyNonCompleteCallstackInfos(
      const ThreadSampleData& actual_thread_sample_data, uint32_t expected_thread_id) {
    EXPECT_EQ(actual_thread_sample_data.thread_id, expected_thread_id);
    EXPECT_EQ(actual_thread_sample_data.samples_count, 5);
    EXPECT_EQ(actual_thread_sample_data.unwinding_errors_count, 5);
    EXPECT_THAT(
        actual_thread_sample_data.sampled_callstack_id_to_events,
        UnorderedElementsAre(
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack1Id, std::vector<CallstackEvent>{{100, kCallstack1Id, kThreadId1},
                                                           {200, kCallstack1Id, kThreadId1}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack2Id, std::vector<CallstackEvent>{{300, kCallstack2Id, kThreadId1}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack3Id, std::vector<CallstackEvent>{{400, kCallstack3Id, kThreadId1}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack4Id, std::vector<CallstackEvent>{{500, kCallstack4Id, kThreadId1}}))));
    EXPECT_THAT(actual_thread_sample_data.sampled_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction3Instruction1AbsoluteAddress, 3),
                                     std::make_pair(kFunction3Instruction2AbsoluteAddress, 1),
                                     std::make_pair(kFunction4Instruction1AbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction3StartAbsoluteAddress, 4),
                                     std::make_pair(kFunction4StartAbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_exclusive_count,
                UnorderedElementsAre(std::make_pair(kFunction3StartAbsoluteAddress, 4),
                                     std::make_pair(kFunction4StartAbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.sorted_count_to_resolved_address,
                UnorderedElementsAre(std::make_pair(4, kFunction3StartAbsoluteAddress),
                                     std::make_pair(1, kFunction4StartAbsoluteAddress)));
    EXPECT_THAT(
        actual_thread_sample_data.sampled_functions,
        UnorderedElementsAre(
            SampledFunctionEq(MakeSampledFunction(kFunction3Name, kModulePath, 4, 80.0f, 4, 80.0f,
                                                  4, 80.0f, kFunction3StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction4Name, kModulePath, 1, 20.0f, 1, 20.0f,
                                                  1, 20.0f, kFunction4StartAbsoluteAddress))));
  }

  static void VerifyThreadSampleDataForCallstackEventsAllInTheSameThreadWithoutAddressInfos(
      const ThreadSampleData& actual_thread_sample_data, uint32_t expected_thread_id) {
    EXPECT_EQ(actual_thread_sample_data.thread_id, expected_thread_id);
    EXPECT_EQ(actual_thread_sample_data.samples_count, 5);
    EXPECT_EQ(actual_thread_sample_data.unwinding_errors_count, 0);
    EXPECT_THAT(
        actual_thread_sample_data.sampled_callstack_id_to_events,
        UnorderedElementsAre(
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack1Id, std::vector<CallstackEvent>{{100, kCallstack1Id, kThreadId1},
                                                           {200, kCallstack1Id, kThreadId1}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack2Id, std::vector<CallstackEvent>{{300, kCallstack2Id, kThreadId1}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack3Id, std::vector<CallstackEvent>{{400, kCallstack3Id, kThreadId1}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack4Id, std::vector<CallstackEvent>{{500, kCallstack4Id, kThreadId1}}))));
    EXPECT_THAT(actual_thread_sample_data.sampled_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1Instruction1AbsoluteAddress, 5),
                                     std::make_pair(kFunction2Instruction1AbsoluteAddress, 3),
                                     std::make_pair(kFunction3Instruction1AbsoluteAddress, 5),
                                     std::make_pair(kFunction3Instruction2AbsoluteAddress, 1),
                                     std::make_pair(kFunction4Instruction1AbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1Instruction1AbsoluteAddress, 5),
                                     std::make_pair(kFunction2Instruction1AbsoluteAddress, 3),
                                     std::make_pair(kFunction3Instruction1AbsoluteAddress, 5),
                                     std::make_pair(kFunction3Instruction2AbsoluteAddress, 1),
                                     std::make_pair(kFunction4Instruction1AbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_exclusive_count,
                UnorderedElementsAre(std::make_pair(kFunction3Instruction1AbsoluteAddress, 3),
                                     std::make_pair(kFunction3Instruction2AbsoluteAddress, 1),
                                     std::make_pair(kFunction4Instruction1AbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.sorted_count_to_resolved_address,
                UnorderedElementsAre(std::make_pair(5, kFunction1Instruction1AbsoluteAddress),
                                     std::make_pair(3, kFunction2Instruction1AbsoluteAddress),
                                     std::make_pair(5, kFunction3Instruction1AbsoluteAddress),
                                     std::make_pair(1, kFunction3Instruction2AbsoluteAddress),
                                     std::make_pair(1, kFunction4Instruction1AbsoluteAddress)));
    EXPECT_THAT(
        actual_thread_sample_data.sampled_functions,
        UnorderedElementsAre(SampledFunctionEq(MakeSampledFunction(
                                 orbit_client_data::kUnknownFunctionOrModuleName,
                                 orbit_client_data::kUnknownFunctionOrModuleName, 0, 0.0f, 5,
                                 100.0f, 0, 0.0f, kFunction1Instruction1AbsoluteAddress)),
                             SampledFunctionEq(MakeSampledFunction(
                                 orbit_client_data::kUnknownFunctionOrModuleName,
                                 orbit_client_data::kUnknownFunctionOrModuleName, 0, 0.0f, 3, 60.0f,
                                 0, 0.0f, kFunction2Instruction1AbsoluteAddress)),
                             SampledFunctionEq(MakeSampledFunction(
                                 orbit_client_data::kUnknownFunctionOrModuleName,
                                 orbit_client_data::kUnknownFunctionOrModuleName, 3, 60.0f, 5,
                                 100.0f, 0, 0.0f, kFunction3Instruction1AbsoluteAddress)),
                             SampledFunctionEq(MakeSampledFunction(
                                 orbit_client_data::kUnknownFunctionOrModuleName,
                                 orbit_client_data::kUnknownFunctionOrModuleName, 1, 20.0f, 1,
                                 20.0f, 0, 0.0f, kFunction3Instruction2AbsoluteAddress)),
                             SampledFunctionEq(MakeSampledFunction(
                                 orbit_client_data::kUnknownFunctionOrModuleName,
                                 orbit_client_data::kUnknownFunctionOrModuleName, 1, 20.0f, 1,
                                 20.0f, 0, 0.0f, kFunction4Instruction1AbsoluteAddress))));
  }

  static void VerifyThreadSampleDataForCallstackEventsInThreadId1(
      const ThreadSampleData& actual_thread_sample_data) {
    EXPECT_EQ(actual_thread_sample_data.thread_id, kThreadId1);
    EXPECT_EQ(actual_thread_sample_data.samples_count, 2);
    EXPECT_EQ(actual_thread_sample_data.unwinding_errors_count, 0);
    EXPECT_THAT(
        actual_thread_sample_data.sampled_callstack_id_to_events,
        UnorderedElementsAre(
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack1Id, std::vector<CallstackEvent>{{100, kCallstack1Id, kThreadId1}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack2Id, std::vector<CallstackEvent>{{200, kCallstack2Id, kThreadId1}}))));
    EXPECT_THAT(actual_thread_sample_data.sampled_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1Instruction1AbsoluteAddress, 2),
                                     std::make_pair(kFunction2Instruction1AbsoluteAddress, 2),
                                     std::make_pair(kFunction3Instruction1AbsoluteAddress, 2),
                                     std::make_pair(kFunction4Instruction1AbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1StartAbsoluteAddress, 2),
                                     std::make_pair(kFunction2StartAbsoluteAddress, 2),
                                     std::make_pair(kFunction3StartAbsoluteAddress, 2),
                                     std::make_pair(kFunction4StartAbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_exclusive_count,
                UnorderedElementsAre(std::make_pair(kFunction3StartAbsoluteAddress, 1),
                                     std::make_pair(kFunction4StartAbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.sorted_count_to_resolved_address,
                UnorderedElementsAre(std::make_pair(2, kFunction1StartAbsoluteAddress),
                                     std::make_pair(2, kFunction2StartAbsoluteAddress),
                                     std::make_pair(2, kFunction3StartAbsoluteAddress),
                                     std::make_pair(1, kFunction4StartAbsoluteAddress)));
    EXPECT_THAT(
        actual_thread_sample_data.sampled_functions,
        UnorderedElementsAre(
            SampledFunctionEq(MakeSampledFunction(kFunction1Name, kModulePath, 0, 0.0f, 2, 100.0f,
                                                  0, 0.0f, kFunction1StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction2Name, kModulePath, 0, 0.0f, 2, 100.0f,
                                                  0, 0.0f, kFunction2StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction3Name, kModulePath, 1, 50.0f, 2, 100.0f,
                                                  0, 0.0f, kFunction3StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction4Name, kModulePath, 1, 50.0f, 1, 50.0f,
                                                  0, 0.0f, kFunction4StartAbsoluteAddress))));
  }

  static void VerifyThreadSampleDataForCallstackEventsInThreadId2(
      const ThreadSampleData& actual_thread_sample_data) {
    EXPECT_EQ(actual_thread_sample_data.thread_id, kThreadId2);
    EXPECT_EQ(actual_thread_sample_data.samples_count, 3);
    EXPECT_EQ(actual_thread_sample_data.unwinding_errors_count, 0);
    EXPECT_THAT(
        actual_thread_sample_data.sampled_callstack_id_to_events,
        UnorderedElementsAre(
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack1Id, std::vector<CallstackEvent>{{300, kCallstack1Id, kThreadId2}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack3Id, std::vector<CallstackEvent>{{400, kCallstack3Id, kThreadId2}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack4Id, std::vector<CallstackEvent>{{500, kCallstack4Id, kThreadId2}}))));
    EXPECT_THAT(actual_thread_sample_data.sampled_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1Instruction1AbsoluteAddress, 3),
                                     std::make_pair(kFunction2Instruction1AbsoluteAddress, 1),
                                     std::make_pair(kFunction3Instruction1AbsoluteAddress, 3),
                                     std::make_pair(kFunction3Instruction2AbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1StartAbsoluteAddress, 3),
                                     std::make_pair(kFunction2StartAbsoluteAddress, 1),
                                     std::make_pair(kFunction3StartAbsoluteAddress, 3)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_exclusive_count,
                ElementsAre(std::make_pair(kFunction3StartAbsoluteAddress, 3)));
    EXPECT_THAT(actual_thread_sample_data.sorted_count_to_resolved_address,
                UnorderedElementsAre(std::make_pair(3, kFunction1StartAbsoluteAddress),
                                     std::make_pair(1, kFunction2StartAbsoluteAddress),
                                     std::make_pair(3, kFunction3StartAbsoluteAddress)));
    EXPECT_THAT(
        actual_thread_sample_data.sampled_functions,
        UnorderedElementsAre(
            SampledFunctionEq(MakeSampledFunction(kFunction1Name, kModulePath, 0, 0.0f, 3, 100.0f,
                                                  0, 0.0f, kFunction1StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction2Name, kModulePath, 0, 0.0f, 1,
                                                  100.0f / 3, 0, 0.0f,
                                                  kFunction2StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction3Name, kModulePath, 3, 100.0f, 3, 100.0f,
                                                  0, 0.0f, kFunction3StartAbsoluteAddress))));
  }

  static void VerifySummaryThreadSampleDataForCallstackEventsInThreadId1And2(
      const ThreadSampleData& actual_thread_sample_data, uint32_t expected_thread_id) {
    EXPECT_EQ(actual_thread_sample_data.thread_id, expected_thread_id);
    EXPECT_THAT(
        actual_thread_sample_data.sampled_callstack_id_to_events,
        UnorderedElementsAre(
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack1Id, std::vector<CallstackEvent>{{100, kCallstack1Id, kThreadId1},
                                                           {300, kCallstack1Id, kThreadId2}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack2Id, std::vector<CallstackEvent>{{200, kCallstack2Id, kThreadId1}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack3Id, std::vector<CallstackEvent>{{400, kCallstack3Id, kThreadId2}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack4Id, std::vector<CallstackEvent>{{500, kCallstack4Id, kThreadId2}}))));
    EXPECT_EQ(actual_thread_sample_data.samples_count, 5);
    EXPECT_EQ(actual_thread_sample_data.unwinding_errors_count, 0);
    EXPECT_THAT(actual_thread_sample_data.sampled_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1Instruction1AbsoluteAddress, 5),
                                     std::make_pair(kFunction2Instruction1AbsoluteAddress, 3),
                                     std::make_pair(kFunction3Instruction1AbsoluteAddress, 5),
                                     std::make_pair(kFunction3Instruction2AbsoluteAddress, 1),
                                     std::make_pair(kFunction4Instruction1AbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1StartAbsoluteAddress, 5),
                                     std::make_pair(kFunction2StartAbsoluteAddress, 3),
                                     std::make_pair(kFunction3StartAbsoluteAddress, 5),
                                     std::make_pair(kFunction4StartAbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_exclusive_count,
                UnorderedElementsAre(std::make_pair(kFunction3StartAbsoluteAddress, 4),
                                     std::make_pair(kFunction4StartAbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.sorted_count_to_resolved_address,
                UnorderedElementsAre(std::make_pair(5, kFunction1StartAbsoluteAddress),
                                     std::make_pair(3, kFunction2StartAbsoluteAddress),
                                     std::make_pair(5, kFunction3StartAbsoluteAddress),
                                     std::make_pair(1, kFunction4StartAbsoluteAddress)));
    EXPECT_THAT(
        actual_thread_sample_data.sampled_functions,
        UnorderedElementsAre(
            SampledFunctionEq(MakeSampledFunction(kFunction1Name, kModulePath, 0, 0.0f, 5, 100.0f,
                                                  0, 0.0f, kFunction1StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction2Name, kModulePath, 0, 0.0f, 3, 60.0f, 0,
                                                  0.0f, kFunction2StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction3Name, kModulePath, 4, 80.0f, 5, 100.0f,
                                                  0, 0.0f, kFunction3StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction4Name, kModulePath, 1, 20.0f, 1, 20.0f,
                                                  0, 0.0f, kFunction4StartAbsoluteAddress))));
  }

  static void VerifyThreadSampleDataForCallstackEventsInThreadId1WithMixedCallstackTypes(
      const ThreadSampleData& actual_thread_sample_data) {
    EXPECT_EQ(actual_thread_sample_data.thread_id, kThreadId1);
    EXPECT_EQ(actual_thread_sample_data.samples_count, 2);
    EXPECT_EQ(actual_thread_sample_data.unwinding_errors_count, 1);
    EXPECT_THAT(
        actual_thread_sample_data.sampled_callstack_id_to_events,
        UnorderedElementsAre(
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack1Id, std::vector<CallstackEvent>{{100, kCallstack1Id, kThreadId1}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack2Id, std::vector<CallstackEvent>{{200, kCallstack2Id, kThreadId1}}))));
    EXPECT_THAT(actual_thread_sample_data.sampled_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1Instruction1AbsoluteAddress, 1),
                                     std::make_pair(kFunction2Instruction1AbsoluteAddress, 1),
                                     std::make_pair(kFunction3Instruction1AbsoluteAddress, 2),
                                     std::make_pair(kFunction4Instruction1AbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1StartAbsoluteAddress, 1),
                                     std::make_pair(kFunction2StartAbsoluteAddress, 1),
                                     std::make_pair(kFunction3StartAbsoluteAddress, 2),
                                     std::make_pair(kFunction4StartAbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_exclusive_count,
                UnorderedElementsAre(std::make_pair(kFunction3StartAbsoluteAddress, 1),
                                     std::make_pair(kFunction4StartAbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.sorted_count_to_resolved_address,
                UnorderedElementsAre(std::make_pair(1, kFunction1StartAbsoluteAddress),
                                     std::make_pair(1, kFunction2StartAbsoluteAddress),
                                     std::make_pair(2, kFunction3StartAbsoluteAddress),
                                     std::make_pair(1, kFunction4StartAbsoluteAddress)));
    EXPECT_THAT(
        actual_thread_sample_data.sampled_functions,
        UnorderedElementsAre(
            SampledFunctionEq(MakeSampledFunction(kFunction1Name, kModulePath, 0, 0.0f, 1, 50.0f, 0,
                                                  0.0f, kFunction1StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction2Name, kModulePath, 0, 0.0f, 1, 50.0f, 0,
                                                  0.0f, kFunction2StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction3Name, kModulePath, 1, 50.0f, 2, 100.0f,
                                                  1, 50.0f, kFunction3StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction4Name, kModulePath, 1, 50.0f, 1, 50.0f,
                                                  0, 0.0f, kFunction4StartAbsoluteAddress))));
  }

  static void VerifyThreadSampleDataForCallstackEventsInThreadId2WithMixedCallstackTypes(
      const ThreadSampleData& actual_thread_sample_data) {
    EXPECT_EQ(actual_thread_sample_data.thread_id, kThreadId2);
    EXPECT_EQ(actual_thread_sample_data.samples_count, 3);
    EXPECT_EQ(actual_thread_sample_data.unwinding_errors_count, 2);
    EXPECT_THAT(
        actual_thread_sample_data.sampled_callstack_id_to_events,
        UnorderedElementsAre(
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack1Id, std::vector<CallstackEvent>{{300, kCallstack1Id, kThreadId2}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack3Id, std::vector<CallstackEvent>{{400, kCallstack3Id, kThreadId2}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack4Id, std::vector<CallstackEvent>{{500, kCallstack4Id, kThreadId2}}))));
    EXPECT_THAT(actual_thread_sample_data.sampled_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1Instruction1AbsoluteAddress, 1),
                                     std::make_pair(kFunction3Instruction1AbsoluteAddress, 3),
                                     std::make_pair(kFunction3Instruction2AbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1StartAbsoluteAddress, 1),
                                     std::make_pair(kFunction3StartAbsoluteAddress, 3)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_exclusive_count,
                ElementsAre(std::make_pair(kFunction3StartAbsoluteAddress, 3)));
    EXPECT_THAT(actual_thread_sample_data.sorted_count_to_resolved_address,
                UnorderedElementsAre(std::make_pair(1, kFunction1StartAbsoluteAddress),
                                     std::make_pair(3, kFunction3StartAbsoluteAddress)));
    EXPECT_THAT(actual_thread_sample_data.sampled_functions,
                UnorderedElementsAre(SampledFunctionEq(MakeSampledFunction(
                                         kFunction1Name, kModulePath, 0, 0.0f, 1, 100.0f / 3, 0,
                                         0.0f, kFunction1StartAbsoluteAddress)),
                                     SampledFunctionEq(MakeSampledFunction(
                                         kFunction3Name, kModulePath, 3, 100.0f, 3, 100.0f, 2,
                                         2 * 100.0f / 3, kFunction3StartAbsoluteAddress))));
  }

  static void VerifyThreadSampleDataForCallstackEventsInThreadId1And2WithMixedCallstackTypes(
      const ThreadSampleData& actual_thread_sample_data, uint32_t expected_thread_id) {
    EXPECT_EQ(actual_thread_sample_data.thread_id, expected_thread_id);
    EXPECT_EQ(actual_thread_sample_data.samples_count, 5);
    EXPECT_EQ(actual_thread_sample_data.unwinding_errors_count, 3);
    EXPECT_THAT(
        actual_thread_sample_data.sampled_callstack_id_to_events,
        UnorderedElementsAre(
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack1Id, std::vector<CallstackEvent>{{100, kCallstack1Id, kThreadId1},
                                                           {300, kCallstack1Id, kThreadId2}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack2Id, std::vector<CallstackEvent>{{200, kCallstack2Id, kThreadId1}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack3Id, std::vector<CallstackEvent>{{400, kCallstack3Id, kThreadId2}})),
            CallstackIdToCallstackEventsEq(std::make_pair(
                kCallstack4Id, std::vector<CallstackEvent>{{500, kCallstack4Id, kThreadId2}}))));
    EXPECT_THAT(actual_thread_sample_data.sampled_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1Instruction1AbsoluteAddress, 2),
                                     std::make_pair(kFunction2Instruction1AbsoluteAddress, 1),
                                     std::make_pair(kFunction3Instruction1AbsoluteAddress, 5),
                                     std::make_pair(kFunction3Instruction2AbsoluteAddress, 1),
                                     std::make_pair(kFunction4Instruction1AbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_count,
                UnorderedElementsAre(std::make_pair(kFunction1StartAbsoluteAddress, 2),
                                     std::make_pair(kFunction2StartAbsoluteAddress, 1),
                                     std::make_pair(kFunction3StartAbsoluteAddress, 5),
                                     std::make_pair(kFunction4StartAbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.resolved_address_to_exclusive_count,
                UnorderedElementsAre(std::make_pair(kFunction3StartAbsoluteAddress, 4),
                                     std::make_pair(kFunction4StartAbsoluteAddress, 1)));
    EXPECT_THAT(actual_thread_sample_data.sorted_count_to_resolved_address,
                UnorderedElementsAre(std::make_pair(2, kFunction1StartAbsoluteAddress),
                                     std::make_pair(1, kFunction2StartAbsoluteAddress),
                                     std::make_pair(5, kFunction3StartAbsoluteAddress),
                                     std::make_pair(1, kFunction4StartAbsoluteAddress)));
    EXPECT_THAT(
        actual_thread_sample_data.sampled_functions,
        UnorderedElementsAre(
            SampledFunctionEq(MakeSampledFunction(kFunction1Name, kModulePath, 0, 0.0f, 2, 40.0f, 0,
                                                  0.0f, kFunction1StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction2Name, kModulePath, 0, 0.0f, 1, 20.0f, 0,
                                                  0.0f, kFunction2StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction3Name, kModulePath, 4, 80.0f, 5, 100.0f,
                                                  3, 60.0f, kFunction3StartAbsoluteAddress)),
            SampledFunctionEq(MakeSampledFunction(kFunction4Name, kModulePath, 1, 20.0f, 1, 20.0f,
                                                  0, 0.0f, kFunction4StartAbsoluteAddress))));
  }

  void VerifyGetCountOfFunction() {
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction1StartAbsoluteAddress), 5);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction1Instruction1AbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction2StartAbsoluteAddress), 3);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction2Instruction1AbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction3StartAbsoluteAddress), 5);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction3Instruction1AbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction3Instruction2AbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction4StartAbsoluteAddress), 1);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction4Instruction1AbsoluteAddress), 0);
  }

  void VerifyGetCountOfFunctionWithOnlyNonCompleteCallstackInfos() {
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction1StartAbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction1Instruction1AbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction2StartAbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction2Instruction1AbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction3StartAbsoluteAddress), 4);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction3Instruction1AbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction3Instruction2AbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction4StartAbsoluteAddress), 1);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction4Instruction1AbsoluteAddress), 0);
  }

  void VerifyGetCountOfFunctionWithMixedCallstackTypes() {
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction1StartAbsoluteAddress), 2);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction1Instruction1AbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction2StartAbsoluteAddress), 1);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction2Instruction1AbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction3StartAbsoluteAddress), 5);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction3Instruction1AbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction3Instruction2AbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction4StartAbsoluteAddress), 1);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction4Instruction1AbsoluteAddress), 0);
  }

  void VerifyGetCountOfFunctionWithoutAddressInfos() {
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction1StartAbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction1Instruction1AbsoluteAddress), 5);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction2StartAbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction2Instruction1AbsoluteAddress), 3);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction3StartAbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction3Instruction1AbsoluteAddress), 5);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction3Instruction2AbsoluteAddress), 1);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction4StartAbsoluteAddress), 0);
    EXPECT_EQ(ppsd_.GetCountOfFunction(kFunction4Instruction1AbsoluteAddress), 1);
  }

  void VerifyEmptySortedCallstackReport(uint32_t thread_id) {
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses({}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1StartAbsoluteAddress, kFunction2StartAbsoluteAddress,
                     kFunction3StartAbsoluteAddress, kFunction4StartAbsoluteAddress},
                    thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    // Also test the non-"Start" addresses.
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction2AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1Instruction1AbsoluteAddress, kFunction2Instruction1AbsoluteAddress,
                     kFunction3Instruction1AbsoluteAddress, kFunction3Instruction2AbsoluteAddress,
                     kFunction4Instruction1AbsoluteAddress},
                    thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
  }

  void VerifySortedCallstackReportForCallstackEventsAllInTheSameThread(uint32_t thread_id) {
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses({}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(
        *ppsd_.GetSortedCallstackReportFromFunctionAddresses({kFunction1StartAbsoluteAddress},
                                                             thread_id),
        SortedCallstackReportEq(MakeSortedCallstackReport(
            {{2, kCallstack1Id}, {1, kCallstack2Id}, {1, kCallstack3Id}, {1, kCallstack4Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(
                    MakeSortedCallstackReport({{2, kCallstack1Id}, {1, kCallstack2Id}})));
    EXPECT_THAT(
        *ppsd_.GetSortedCallstackReportFromFunctionAddresses({kFunction3StartAbsoluteAddress},
                                                             thread_id),
        SortedCallstackReportEq(MakeSortedCallstackReport(
            {{2, kCallstack1Id}, {1, kCallstack2Id}, {1, kCallstack3Id}, {1, kCallstack4Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({{1, kCallstack2Id}})));

    EXPECT_THAT(
        *ppsd_.GetSortedCallstackReportFromFunctionAddresses(
            {kFunction1StartAbsoluteAddress, kFunction2StartAbsoluteAddress,
             kFunction3StartAbsoluteAddress, kFunction4StartAbsoluteAddress},
            thread_id),
        SortedCallstackReportEq(MakeSortedCallstackReport(
            {{2, kCallstack1Id}, {1, kCallstack2Id}, {1, kCallstack3Id}, {1, kCallstack4Id}})));

    // Also test the non-"Start" addresses.
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction2AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1Instruction1AbsoluteAddress, kFunction2Instruction1AbsoluteAddress,
                     kFunction3Instruction1AbsoluteAddress, kFunction3Instruction2AbsoluteAddress,
                     kFunction4Instruction1AbsoluteAddress},
                    thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
  }

  void
  VerifySortedCallstackReportForCallstackEventsAllInTheSameThreadWithOnlyNonCompleteCallstackInfos(
      uint32_t thread_id) {
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses({}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport(
                    {{2, kCallstack1Id}, {1, kCallstack3Id}, {1, kCallstack4Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({{1, kCallstack2Id}})));

    EXPECT_THAT(
        *ppsd_.GetSortedCallstackReportFromFunctionAddresses(
            {kFunction1StartAbsoluteAddress, kFunction2StartAbsoluteAddress,
             kFunction3StartAbsoluteAddress, kFunction4StartAbsoluteAddress},
            thread_id),
        SortedCallstackReportEq(MakeSortedCallstackReport(
            {{2, kCallstack1Id}, {1, kCallstack2Id}, {1, kCallstack3Id}, {1, kCallstack4Id}})));

    // Also test the non-"Start" addresses.
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction2AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1Instruction1AbsoluteAddress, kFunction2Instruction1AbsoluteAddress,
                     kFunction3Instruction1AbsoluteAddress, kFunction3Instruction2AbsoluteAddress,
                     kFunction4Instruction1AbsoluteAddress},
                    thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
  }

  void VerifySortedCallstackReportForCallstackEventsAllInTheSameThreadWithMixedCallstackTypes(
      uint32_t thread_id) {
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses({}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(
                    MakeSortedCallstackReport({{1, kCallstack2Id}, {1, kCallstack3Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({{1, kCallstack2Id}})));
    EXPECT_THAT(
        *ppsd_.GetSortedCallstackReportFromFunctionAddresses({kFunction3StartAbsoluteAddress},
                                                             thread_id),
        SortedCallstackReportEq(MakeSortedCallstackReport(
            {{2, kCallstack1Id}, {1, kCallstack2Id}, {1, kCallstack3Id}, {1, kCallstack4Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({{1, kCallstack2Id}})));

    EXPECT_THAT(
        *ppsd_.GetSortedCallstackReportFromFunctionAddresses(
            {kFunction1StartAbsoluteAddress, kFunction2StartAbsoluteAddress,
             kFunction3StartAbsoluteAddress, kFunction4StartAbsoluteAddress},
            thread_id),
        SortedCallstackReportEq(MakeSortedCallstackReport(
            {{2, kCallstack1Id}, {1, kCallstack2Id}, {1, kCallstack3Id}, {1, kCallstack4Id}})));

    // Also test the non-"Start" addresses.
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction2AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1Instruction1AbsoluteAddress, kFunction2Instruction1AbsoluteAddress,
                     kFunction3Instruction1AbsoluteAddress, kFunction3Instruction2AbsoluteAddress,
                     kFunction4Instruction1AbsoluteAddress},
                    thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
  }

  void VerifySortedCallstackReportForCallstackEventsAllInTheSameThreadWithoutAddressInfo(
      uint32_t thread_id) {
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses({}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(
        *ppsd_.GetSortedCallstackReportFromFunctionAddresses(
            {kFunction1Instruction1AbsoluteAddress}, thread_id),
        SortedCallstackReportEq(MakeSortedCallstackReport(
            {{2, kCallstack1Id}, {1, kCallstack2Id}, {1, kCallstack3Id}, {1, kCallstack4Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(
                    MakeSortedCallstackReport({{2, kCallstack1Id}, {1, kCallstack2Id}})));
    EXPECT_THAT(
        *ppsd_.GetSortedCallstackReportFromFunctionAddresses(
            {kFunction3Instruction1AbsoluteAddress}, thread_id),
        SortedCallstackReportEq(MakeSortedCallstackReport(
            {{2, kCallstack1Id}, {1, kCallstack2Id}, {1, kCallstack3Id}, {1, kCallstack4Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction2AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({{1, kCallstack3Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({{1, kCallstack2Id}})));

    EXPECT_THAT(
        *ppsd_.GetSortedCallstackReportFromFunctionAddresses(
            {kFunction1Instruction1AbsoluteAddress, kFunction2Instruction1AbsoluteAddress,
             kFunction3Instruction1AbsoluteAddress, kFunction3Instruction2AbsoluteAddress,
             kFunction4Instruction1AbsoluteAddress},
            thread_id),
        SortedCallstackReportEq(MakeSortedCallstackReport(
            {{2, kCallstack1Id}, {1, kCallstack2Id}, {1, kCallstack3Id}, {1, kCallstack4Id}})));

    // Also test the "Start" addresses.
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1StartAbsoluteAddress, kFunction2StartAbsoluteAddress,
                     kFunction3StartAbsoluteAddress, kFunction4StartAbsoluteAddress},
                    thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
  }

  void VerifySortedCallstackReportForCallstackEventsInThreadId1() {
    uint32_t thread_id = kThreadId1;

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses({}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(
                    MakeSortedCallstackReport({{1, kCallstack1Id}, {1, kCallstack2Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(
                    MakeSortedCallstackReport({{1, kCallstack1Id}, {1, kCallstack2Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(
                    MakeSortedCallstackReport({{1, kCallstack1Id}, {1, kCallstack2Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({{1, kCallstack2Id}})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1StartAbsoluteAddress, kFunction2StartAbsoluteAddress,
                     kFunction3StartAbsoluteAddress, kFunction4StartAbsoluteAddress},
                    thread_id),
                SortedCallstackReportEq(
                    MakeSortedCallstackReport({{1, kCallstack1Id}, {1, kCallstack2Id}})));

    // Also test the non-"Start" addresses.
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction2AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1Instruction1AbsoluteAddress, kFunction2Instruction1AbsoluteAddress,
                     kFunction3Instruction1AbsoluteAddress, kFunction3Instruction2AbsoluteAddress,
                     kFunction4Instruction1AbsoluteAddress},
                    thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
  }

  void VerifySortedCallstackReportForCallstackEventsInThreadId2() {
    uint32_t thread_id = kThreadId2;

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses({}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport(
                    {{1, kCallstack1Id}, {1, kCallstack3Id}, {1, kCallstack4Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({{1, kCallstack1Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport(
                    {{1, kCallstack1Id}, {1, kCallstack3Id}, {1, kCallstack4Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1StartAbsoluteAddress, kFunction2StartAbsoluteAddress,
                     kFunction3StartAbsoluteAddress, kFunction4StartAbsoluteAddress},
                    thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport(
                    {{1, kCallstack1Id}, {1, kCallstack3Id}, {1, kCallstack4Id}})));

    // Also test the non-"Start" addresses.
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction2AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1Instruction1AbsoluteAddress, kFunction2Instruction1AbsoluteAddress,
                     kFunction3Instruction1AbsoluteAddress, kFunction3Instruction2AbsoluteAddress,
                     kFunction4Instruction1AbsoluteAddress},
                    thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
  }

  void VerifySortedCallstackReportForCallstackEventsInThreadId1WithMixedCallstackTypes() {
    uint32_t thread_id = kThreadId1;

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses({}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({{1, kCallstack2Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({{1, kCallstack2Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(
                    MakeSortedCallstackReport({{1, kCallstack1Id}, {1, kCallstack2Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({{1, kCallstack2Id}})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1StartAbsoluteAddress, kFunction2StartAbsoluteAddress,
                     kFunction3StartAbsoluteAddress, kFunction4StartAbsoluteAddress},
                    thread_id),
                SortedCallstackReportEq(
                    MakeSortedCallstackReport({{1, kCallstack1Id}, {1, kCallstack2Id}})));

    // Also test the non-"Start" addresses.
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction2AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1Instruction1AbsoluteAddress, kFunction2Instruction1AbsoluteAddress,
                     kFunction3Instruction1AbsoluteAddress, kFunction3Instruction2AbsoluteAddress,
                     kFunction4Instruction1AbsoluteAddress},
                    thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
  }

  void VerifySortedCallstackReportForCallstackEventsInThreadId2WithMixedCallstackTypes() {
    uint32_t thread_id = kThreadId2;

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses({}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({{1, kCallstack3Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport(
                    {{1, kCallstack1Id}, {1, kCallstack3Id}, {1, kCallstack4Id}})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4StartAbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1StartAbsoluteAddress, kFunction2StartAbsoluteAddress,
                     kFunction3StartAbsoluteAddress, kFunction4StartAbsoluteAddress},
                    thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport(
                    {{1, kCallstack1Id}, {1, kCallstack3Id}, {1, kCallstack4Id}})));

    // Also test the non-"Start" addresses.
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction2Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction3Instruction2AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction4Instruction1AbsoluteAddress}, thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));

    EXPECT_THAT(*ppsd_.GetSortedCallstackReportFromFunctionAddresses(
                    {kFunction1Instruction1AbsoluteAddress, kFunction2Instruction1AbsoluteAddress,
                     kFunction3Instruction1AbsoluteAddress, kFunction3Instruction2AbsoluteAddress,
                     kFunction4Instruction1AbsoluteAddress},
                    thread_id),
                SortedCallstackReportEq(MakeSortedCallstackReport({})));
  }

 private:
  uint64_t current_callstack_timestamp_ns_ = 0;
};

}  // namespace

TEST_F(SamplingDataPostProcessorTest, EmptyCallstackDataWithoutSummaryWithoutEvenAddressInfos) {
  CreatePostProcessedSamplingDataWithoutSummary();

  VerifyNoCallstackInfos();

  EXPECT_EQ(ppsd_.GetSortedThreadSampleData().size(), 0);
  EXPECT_EQ(ppsd_.GetSummary(), nullptr);

  VerifyEmptySortedCallstackReport(orbit_base::kAllProcessThreadsTid);
  VerifyEmptySortedCallstackReport(kThreadIdNotSampled);
}

TEST_F(SamplingDataPostProcessorTest, EmptyCallstackDataWithSummaryWithoutEvenAddressInfos) {
  CreatePostProcessedSamplingDataWithSummary();

  VerifyNoCallstackInfos();

  EXPECT_EQ(ppsd_.GetSortedThreadSampleData().size(), 0);
  EXPECT_EQ(ppsd_.GetSummary(), nullptr);

  VerifyEmptySortedCallstackReport(orbit_base::kAllProcessThreadsTid);
  VerifyEmptySortedCallstackReport(kThreadIdNotSampled);
}

TEST_F(SamplingDataPostProcessorTest, EmptyCallstackDataWithoutSummary) {
  AddAllAddressInfos();

  CreatePostProcessedSamplingDataWithoutSummary();

  VerifyNoCallstackInfos();

  EXPECT_EQ(ppsd_.GetSortedThreadSampleData().size(), 0);
  EXPECT_EQ(ppsd_.GetSummary(), nullptr);

  VerifyEmptySortedCallstackReport(orbit_base::kAllProcessThreadsTid);
  VerifyEmptySortedCallstackReport(kThreadIdNotSampled);
}

TEST_F(SamplingDataPostProcessorTest, EmptyCallstackDataWithSummary) {
  AddAllAddressInfos();

  CreatePostProcessedSamplingDataWithSummary();

  VerifyNoCallstackInfos();

  EXPECT_EQ(ppsd_.GetSortedThreadSampleData().size(), 0);
  EXPECT_EQ(ppsd_.GetSummary(), nullptr);

  VerifyEmptySortedCallstackReport(orbit_base::kAllProcessThreadsTid);
  VerifyEmptySortedCallstackReport(kThreadIdNotSampled);
}

TEST_F(SamplingDataPostProcessorTest, CallstackEventOfEmptyCallstack) {
  AddAllAddressInfos();

  static constexpr uint64_t kEmptyCallstackId = 99;
  AddCallstackInfo(kEmptyCallstackId, {}, CallstackType::kComplete);
  AddCallstackEvent(kEmptyCallstackId, kThreadId1);

  EXPECT_DEATH(CreatePostProcessedSamplingDataWithSummary(), "Check failed");
}

TEST_F(SamplingDataPostProcessorTest, CallstackInfosButNoCallstackEvents) {
  AddAllAddressInfos();
  AddAllCallstackInfos(CallstackType::kComplete);

  CreatePostProcessedSamplingDataWithSummary();

  VerifyAllCallstackInfos(CallstackType::kComplete);

  EXPECT_EQ(ppsd_.GetSortedThreadSampleData().size(), 0);
  EXPECT_EQ(ppsd_.GetSummary(), nullptr);

  VerifyEmptySortedCallstackReport(orbit_base::kAllProcessThreadsTid);
  VerifyEmptySortedCallstackReport(kThreadIdNotSampled);
}

TEST_F(SamplingDataPostProcessorTest, OneThreadWithoutSummary) {
  AddAllCallstackInfos(CallstackType::kComplete);
  AddAllAddressInfos();

  AddCallstackEventsAllInThreadId1();

  CreatePostProcessedSamplingDataWithoutSummary();

  VerifyAllCallstackInfos(CallstackType::kComplete);

  EXPECT_EQ(ppsd_.GetSortedThreadSampleData().size(), 1);
  EXPECT_EQ(ppsd_.GetSummary(), nullptr);

  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(kThreadId1), nullptr);
  EXPECT_THAT(ppsd_.GetSortedThreadSampleData(),
              ElementsAre(ppsd_.GetThreadSampleDataByThreadId(kThreadId1)));

  VerifyThreadSampleDataForCallstackEventsAllInTheSameThread(
      *ppsd_.GetThreadSampleDataByThreadId(kThreadId1), kThreadId1);

  VerifyGetCountOfFunction();

  VerifyEmptySortedCallstackReport(orbit_base::kAllProcessThreadsTid);
  VerifySortedCallstackReportForCallstackEventsAllInTheSameThread(kThreadId1);
  VerifyEmptySortedCallstackReport(kThreadIdNotSampled);
}

TEST_F(SamplingDataPostProcessorTest, OneThreadWithSummary) {
  AddAllCallstackInfos(CallstackType::kComplete);
  AddAllAddressInfos();

  AddCallstackEventsAllInThreadId1();

  CreatePostProcessedSamplingDataWithSummary();

  VerifyAllCallstackInfos(CallstackType::kComplete);

  EXPECT_EQ(ppsd_.GetSortedThreadSampleData().size(), 2);
  ASSERT_NE(ppsd_.GetSummary(), nullptr);

  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(orbit_base::kAllProcessThreadsTid), nullptr);
  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(kThreadId1), nullptr);
  EXPECT_EQ(ppsd_.GetSummary(),
            ppsd_.GetThreadSampleDataByThreadId(orbit_base::kAllProcessThreadsTid));
  EXPECT_THAT(ppsd_.GetSortedThreadSampleData(),
              ElementsAre(ppsd_.GetSummary(), ppsd_.GetThreadSampleDataByThreadId(kThreadId1)));

  VerifyThreadSampleDataForCallstackEventsAllInTheSameThread(*ppsd_.GetSummary(),
                                                             orbit_base::kAllProcessThreadsTid);
  VerifyThreadSampleDataForCallstackEventsAllInTheSameThread(
      *ppsd_.GetThreadSampleDataByThreadId(kThreadId1), kThreadId1);

  VerifyGetCountOfFunction();

  VerifySortedCallstackReportForCallstackEventsAllInTheSameThread(
      orbit_base::kAllProcessThreadsTid);
  VerifySortedCallstackReportForCallstackEventsAllInTheSameThread(kThreadId1);
  VerifyEmptySortedCallstackReport(kThreadIdNotSampled);
}

TEST_F(SamplingDataPostProcessorTest, OneThreadWithSummaryWithOnlyNonCompleteCallstackInfos) {
  AddAllAddressInfos();
  AddAllCallstackInfos(CallstackType::kDwarfUnwindingError);

  AddCallstackEventsAllInThreadId1();

  CreatePostProcessedSamplingDataWithSummary();

  VerifyAllCallstackInfos(CallstackType::kDwarfUnwindingError);

  EXPECT_EQ(ppsd_.GetSortedThreadSampleData().size(), 2);
  ASSERT_NE(ppsd_.GetSummary(), nullptr);

  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(orbit_base::kAllProcessThreadsTid), nullptr);
  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(kThreadId1), nullptr);
  EXPECT_EQ(ppsd_.GetSummary(),
            ppsd_.GetThreadSampleDataByThreadId(orbit_base::kAllProcessThreadsTid));
  EXPECT_THAT(ppsd_.GetSortedThreadSampleData(),
              ElementsAre(ppsd_.GetSummary(), ppsd_.GetThreadSampleDataByThreadId(kThreadId1)));

  VerifyThreadSampleDataForCallstackEventsAllInTheSameThreadWithOnlyNonCompleteCallstackInfos(
      *ppsd_.GetSummary(), orbit_base::kAllProcessThreadsTid);
  VerifyThreadSampleDataForCallstackEventsAllInTheSameThreadWithOnlyNonCompleteCallstackInfos(
      *ppsd_.GetThreadSampleDataByThreadId(kThreadId1), kThreadId1);

  VerifyGetCountOfFunctionWithOnlyNonCompleteCallstackInfos();

  VerifySortedCallstackReportForCallstackEventsAllInTheSameThreadWithOnlyNonCompleteCallstackInfos(
      orbit_base::kAllProcessThreadsTid);
  VerifySortedCallstackReportForCallstackEventsAllInTheSameThreadWithOnlyNonCompleteCallstackInfos(
      kThreadId1);
  VerifyEmptySortedCallstackReport(kThreadIdNotSampled);
}

TEST_F(SamplingDataPostProcessorTest, OneThreadWithoutSummaryWithMixedCallstackTypes) {
  AddAllCallstackInfosWithMixedCallstackTypes();
  AddAllAddressInfos();

  AddCallstackEventsAllInThreadId1();

  CreatePostProcessedSamplingDataWithoutSummary();

  VerifyAllCallstackInfosWithMixedCallstackTypes();

  EXPECT_EQ(ppsd_.GetSortedThreadSampleData().size(), 1);
  EXPECT_EQ(ppsd_.GetSummary(), nullptr);

  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(kThreadId1), nullptr);
  EXPECT_THAT(ppsd_.GetSortedThreadSampleData(),
              ElementsAre(ppsd_.GetThreadSampleDataByThreadId(kThreadId1)));

  VerifyThreadSampleDataForCallstackEventsAllInTheSameThreadWithMixedCallstackTypes(
      *ppsd_.GetThreadSampleDataByThreadId(kThreadId1), kThreadId1);

  VerifyGetCountOfFunctionWithMixedCallstackTypes();

  VerifyEmptySortedCallstackReport(orbit_base::kAllProcessThreadsTid);
  VerifySortedCallstackReportForCallstackEventsAllInTheSameThreadWithMixedCallstackTypes(
      kThreadId1);
  VerifyEmptySortedCallstackReport(kThreadIdNotSampled);
}

TEST_F(SamplingDataPostProcessorTest, OneThreadWithSummaryWithMixedCallstackTypes) {
  AddAllCallstackInfosWithMixedCallstackTypes();
  AddAllAddressInfos();

  AddCallstackEventsAllInThreadId1();

  CreatePostProcessedSamplingDataWithSummary();

  VerifyAllCallstackInfosWithMixedCallstackTypes();

  EXPECT_EQ(ppsd_.GetSortedThreadSampleData().size(), 2);
  ASSERT_NE(ppsd_.GetSummary(), nullptr);

  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(orbit_base::kAllProcessThreadsTid), nullptr);
  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(kThreadId1), nullptr);
  EXPECT_EQ(ppsd_.GetSummary(),
            ppsd_.GetThreadSampleDataByThreadId(orbit_base::kAllProcessThreadsTid));
  EXPECT_THAT(ppsd_.GetSortedThreadSampleData(),
              ElementsAre(ppsd_.GetSummary(), ppsd_.GetThreadSampleDataByThreadId(kThreadId1)));

  VerifyThreadSampleDataForCallstackEventsAllInTheSameThreadWithMixedCallstackTypes(
      *ppsd_.GetSummary(), orbit_base::kAllProcessThreadsTid);
  VerifyThreadSampleDataForCallstackEventsAllInTheSameThreadWithMixedCallstackTypes(
      *ppsd_.GetThreadSampleDataByThreadId(kThreadId1), kThreadId1);

  VerifyGetCountOfFunctionWithMixedCallstackTypes();

  VerifySortedCallstackReportForCallstackEventsAllInTheSameThreadWithMixedCallstackTypes(
      orbit_base::kAllProcessThreadsTid);
  VerifySortedCallstackReportForCallstackEventsAllInTheSameThreadWithMixedCallstackTypes(
      kThreadId1);
  VerifyEmptySortedCallstackReport(kThreadIdNotSampled);
}

// This test shows what happens when each different address is considered a different function.
TEST_F(SamplingDataPostProcessorTest, OneThreadWithSummaryWithoutAddressInfos) {
  AddAllCallstackInfos(CallstackType::kComplete);

  AddCallstackEventsAllInThreadId1();

  CreatePostProcessedSamplingDataWithSummary();

  VerifyAllCallstackInfosWithoutAddressInfos(CallstackType::kComplete);

  EXPECT_EQ(ppsd_.GetSortedThreadSampleData().size(), 2);
  ASSERT_NE(ppsd_.GetSummary(), nullptr);

  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(orbit_base::kAllProcessThreadsTid), nullptr);
  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(kThreadId1), nullptr);
  EXPECT_EQ(ppsd_.GetSummary(),
            ppsd_.GetThreadSampleDataByThreadId(orbit_base::kAllProcessThreadsTid));
  EXPECT_THAT(ppsd_.GetSortedThreadSampleData(),
              ElementsAre(ppsd_.GetSummary(), ppsd_.GetThreadSampleDataByThreadId(kThreadId1)));

  VerifyThreadSampleDataForCallstackEventsAllInTheSameThreadWithoutAddressInfos(
      *ppsd_.GetSummary(), orbit_base::kAllProcessThreadsTid);
  VerifyThreadSampleDataForCallstackEventsAllInTheSameThreadWithoutAddressInfos(
      *ppsd_.GetThreadSampleDataByThreadId(kThreadId1), kThreadId1);

  VerifyGetCountOfFunctionWithoutAddressInfos();

  VerifySortedCallstackReportForCallstackEventsAllInTheSameThreadWithoutAddressInfo(
      orbit_base::kAllProcessThreadsTid);
  VerifySortedCallstackReportForCallstackEventsAllInTheSameThreadWithoutAddressInfo(kThreadId1);
  VerifyEmptySortedCallstackReport(kThreadIdNotSampled);
}

TEST_F(SamplingDataPostProcessorTest, TwoThreadsWithoutSummary) {
  AddAllCallstackInfos(CallstackType::kComplete);
  AddAllAddressInfos();

  AddCallstackEventsInThreadId1And2();

  CreatePostProcessedSamplingDataWithoutSummary();

  VerifyAllCallstackInfos(CallstackType::kComplete);

  EXPECT_EQ(ppsd_.GetSortedThreadSampleData().size(), 2);
  EXPECT_EQ(ppsd_.GetSummary(), nullptr);

  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(kThreadId1), nullptr);
  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(kThreadId2), nullptr);
  EXPECT_THAT(ppsd_.GetSortedThreadSampleData(),
              ElementsAre(ppsd_.GetThreadSampleDataByThreadId(kThreadId2),
                          ppsd_.GetThreadSampleDataByThreadId(kThreadId1)));

  VerifyThreadSampleDataForCallstackEventsInThreadId1(
      *ppsd_.GetThreadSampleDataByThreadId(kThreadId1));
  VerifyThreadSampleDataForCallstackEventsInThreadId2(
      *ppsd_.GetThreadSampleDataByThreadId(kThreadId2));

  VerifyGetCountOfFunction();

  VerifyEmptySortedCallstackReport(orbit_base::kAllProcessThreadsTid);
  VerifySortedCallstackReportForCallstackEventsInThreadId1();
  VerifySortedCallstackReportForCallstackEventsInThreadId2();
  VerifyEmptySortedCallstackReport(kThreadIdNotSampled);
}

TEST_F(SamplingDataPostProcessorTest, TwoThreadsWithSummary) {
  AddAllCallstackInfos(CallstackType::kComplete);
  AddAllAddressInfos();

  AddCallstackEventsInThreadId1And2();

  CreatePostProcessedSamplingDataWithSummary();

  VerifyAllCallstackInfos(CallstackType::kComplete);

  EXPECT_EQ(ppsd_.GetSortedThreadSampleData().size(), 3);
  ASSERT_NE(ppsd_.GetSummary(), nullptr);

  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(kThreadId1), nullptr);
  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(kThreadId2), nullptr);
  EXPECT_THAT(ppsd_.GetSortedThreadSampleData(),
              ElementsAre(ppsd_.GetSummary(), ppsd_.GetThreadSampleDataByThreadId(kThreadId2),
                          ppsd_.GetThreadSampleDataByThreadId(kThreadId1)));

  VerifySummaryThreadSampleDataForCallstackEventsInThreadId1And2(*ppsd_.GetSummary(),
                                                                 orbit_base::kAllProcessThreadsTid);
  VerifyThreadSampleDataForCallstackEventsInThreadId1(
      *ppsd_.GetThreadSampleDataByThreadId(kThreadId1));
  VerifyThreadSampleDataForCallstackEventsInThreadId2(
      *ppsd_.GetThreadSampleDataByThreadId(kThreadId2));

  VerifyGetCountOfFunction();

  VerifySortedCallstackReportForCallstackEventsAllInTheSameThread(
      orbit_base::kAllProcessThreadsTid);
  VerifySortedCallstackReportForCallstackEventsInThreadId1();
  VerifySortedCallstackReportForCallstackEventsInThreadId2();
  VerifyEmptySortedCallstackReport(kThreadIdNotSampled);
}

TEST_F(SamplingDataPostProcessorTest, TwoThreadsWithoutSummaryWithMixedCallstackTypes) {
  AddAllCallstackInfosWithMixedCallstackTypes();
  AddAllAddressInfos();

  AddCallstackEventsInThreadId1And2();

  CreatePostProcessedSamplingDataWithoutSummary();

  VerifyAllCallstackInfosWithMixedCallstackTypes();

  EXPECT_EQ(ppsd_.GetSortedThreadSampleData().size(), 2);
  EXPECT_EQ(ppsd_.GetSummary(), nullptr);

  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(kThreadId1), nullptr);
  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(kThreadId2), nullptr);
  EXPECT_THAT(ppsd_.GetSortedThreadSampleData(),
              ElementsAre(ppsd_.GetThreadSampleDataByThreadId(kThreadId2),
                          ppsd_.GetThreadSampleDataByThreadId(kThreadId1)));

  VerifyThreadSampleDataForCallstackEventsInThreadId1WithMixedCallstackTypes(
      *ppsd_.GetThreadSampleDataByThreadId(kThreadId1));
  VerifyThreadSampleDataForCallstackEventsInThreadId2WithMixedCallstackTypes(
      *ppsd_.GetThreadSampleDataByThreadId(kThreadId2));

  VerifyGetCountOfFunctionWithMixedCallstackTypes();

  VerifyEmptySortedCallstackReport(orbit_base::kAllProcessThreadsTid);
  VerifySortedCallstackReportForCallstackEventsInThreadId1WithMixedCallstackTypes();
  VerifySortedCallstackReportForCallstackEventsInThreadId2WithMixedCallstackTypes();
  VerifyEmptySortedCallstackReport(kThreadIdNotSampled);
}

TEST_F(SamplingDataPostProcessorTest, TwoThreadsWithSummaryWithMixedCallstackTypes) {
  AddAllCallstackInfosWithMixedCallstackTypes();
  AddAllAddressInfos();

  AddCallstackEventsInThreadId1And2();

  CreatePostProcessedSamplingDataWithSummary();

  VerifyAllCallstackInfosWithMixedCallstackTypes();

  EXPECT_EQ(ppsd_.GetSortedThreadSampleData().size(), 3);
  ASSERT_NE(ppsd_.GetSummary(), nullptr);

  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(kThreadId1), nullptr);
  ASSERT_NE(ppsd_.GetThreadSampleDataByThreadId(kThreadId2), nullptr);
  EXPECT_THAT(ppsd_.GetSortedThreadSampleData(),
              ElementsAre(ppsd_.GetSummary(), ppsd_.GetThreadSampleDataByThreadId(kThreadId2),
                          ppsd_.GetThreadSampleDataByThreadId(kThreadId1)));

  VerifyThreadSampleDataForCallstackEventsInThreadId1And2WithMixedCallstackTypes(
      *ppsd_.GetSummary(), orbit_base::kAllProcessThreadsTid);
  VerifyThreadSampleDataForCallstackEventsInThreadId1WithMixedCallstackTypes(
      *ppsd_.GetThreadSampleDataByThreadId(kThreadId1));
  VerifyThreadSampleDataForCallstackEventsInThreadId2WithMixedCallstackTypes(
      *ppsd_.GetThreadSampleDataByThreadId(kThreadId2));

  VerifyGetCountOfFunctionWithMixedCallstackTypes();

  VerifySortedCallstackReportForCallstackEventsAllInTheSameThreadWithMixedCallstackTypes(
      orbit_base::kAllProcessThreadsTid);
  VerifySortedCallstackReportForCallstackEventsInThreadId1WithMixedCallstackTypes();
  VerifySortedCallstackReportForCallstackEventsInThreadId2WithMixedCallstackTypes();
  VerifyEmptySortedCallstackReport(kThreadIdNotSampled);
}

}  // namespace orbit_client_model
