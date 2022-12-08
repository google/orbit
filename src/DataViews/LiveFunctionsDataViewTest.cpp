// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/algorithm/container.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <absl/strings/str_format.h>
#include <absl/time/time.h>
#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ClientData/CaptureData.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/MockScopeStatsCollection.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ScopeId.h"
#include "ClientData/ScopeStats.h"
#include "ClientData/ThreadTrackDataProvider.h"
#include "ClientData/TimerChain.h"
#include "ClientProtos/capture_data.pb.h"
#include "DataViewTestUtils.h"
#include "DataViews/AppInterface.h"
#include "DataViews/DataView.h"
#include "DataViews/FunctionsDataView.h"
#include "DataViews/LiveFunctionsDataView.h"
#include "DataViews/LiveFunctionsInterface.h"
#include "DisplayFormats/DisplayFormats.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "MockAppInterface.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Typedef.h"
#include "SymbolProvider/ModuleIdentifier.h"

using JumpToTimerMode = orbit_data_views::AppInterface::JumpToTimerMode;

using orbit_client_data::CaptureData;
using orbit_client_data::FunctionInfo;
using orbit_client_data::MockScopeStatsCollection;
using orbit_client_data::ScopeId;
using orbit_client_data::ScopeStats;

using orbit_client_protos::TimerInfo;

using orbit_data_views::CheckCopySelectionIsInvoked;
using orbit_data_views::CheckExportToCsvIsInvoked;
using orbit_data_views::CheckSingleAction;
using orbit_data_views::ContextMenuEntry;
using orbit_data_views::FlattenContextMenu;
using orbit_data_views::FlattenContextMenuWithGroupingAndCheckOrder;
using orbit_data_views::GetActionIndexOnMenu;
using orbit_data_views::kInvalidActionIndex;
using orbit_data_views::kMenuActionAddIterator;
using orbit_data_views::kMenuActionCopySelection;
using orbit_data_views::kMenuActionDisableFrameTrack;
using orbit_data_views::kMenuActionDisassembly;
using orbit_data_views::kMenuActionEnableFrameTrack;
using orbit_data_views::kMenuActionExportEventsToCsv;
using orbit_data_views::kMenuActionExportToCsv;
using orbit_data_views::kMenuActionJumpToFirst;
using orbit_data_views::kMenuActionJumpToLast;
using orbit_data_views::kMenuActionJumpToMax;
using orbit_data_views::kMenuActionJumpToMin;
using orbit_data_views::kMenuActionSelect;
using orbit_data_views::kMenuActionSourceCode;
using orbit_data_views::kMenuActionUnselect;

using orbit_grpc_protos::InstrumentedFunction;
using orbit_grpc_protos::ModuleInfo;

using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;

namespace {

constexpr size_t kNumFunctions = 3;
constexpr std::array<uint64_t, kNumFunctions> kFunctionIds{11, 22, 33};
const std::array<ScopeId, kNumFunctions> kScopeIds = [] {
  std::array<ScopeId, kNumFunctions> result;
  std::transform(std::begin(kFunctionIds), std::end(kFunctionIds), std::begin(result),
                 [](uint64_t scope_id) { return ScopeId(scope_id); });
  return result;
}();
constexpr ScopeId kNonDynamicallyInstrumentedFunctionId{123456};
const std::array<std::string, kNumFunctions> kPrettyNames{"void foo()", "main(int, char**)",
                                                          "ffind(int)"};
const std::array<std::string, kNumFunctions> kModulePaths{
    "/path/to/foomodule", "/path/to/somemodule", "/path/to/ffindmodule"};
constexpr std::array<uint64_t, kNumFunctions> kAddresses{0x300, 0x100, 0x200};
constexpr std::array<uint64_t, kNumFunctions> kSizes{111, 222, 333};
constexpr std::array<uint64_t, kNumFunctions> kLoadBiases{0x10, 0x20, 0x30};
const std::array<std::string, kNumFunctions> kBuildIds{"build_id_0", "build_id_1", "build_id_2"};

constexpr std::array<uint64_t, kNumFunctions> kCounts{150, 30, 0};
constexpr std::array<uint64_t, kNumFunctions> kTotalTimeNs{450000, 300000, 0};
constexpr std::array<uint64_t, kNumFunctions> kAvgTimeNs{3000, 10000, 0};
constexpr std::array<uint64_t, kNumFunctions> kMinNs{2000, 3000, 0};
constexpr std::array<uint64_t, kNumFunctions> kMaxNs{4000, 12000, 0};
constexpr std::array<uint64_t, kNumFunctions> kStdDevNs{1000, 6000, 0};

constexpr int kColumnSelected = 0;
constexpr int kColumnName = 1;
constexpr int kColumnCount = 2;
constexpr int kColumnTimeTotal = 3;
constexpr int kColumnTimeAvg = 4;
constexpr int kColumnTimeMin = 5;
constexpr int kColumnTimeMax = 6;
constexpr int kColumnStdDev = 7;
constexpr int kColumnModule = 8;
constexpr int kColumnAddress = 9;
constexpr int kNumColumns = 10;

constexpr size_t kNumThreads = 2;
constexpr std::array<uint32_t, kNumThreads> kThreadIds = {111, 222};
const std::array<std::string, kNumThreads> kThreadNames = {"Test Thread 1", "Test Thread 2"};

constexpr size_t kNumTimers = 3;
constexpr std::array<uint64_t, kNumTimers> kStarts = {1000, 2345, 6789};
constexpr std::array<uint64_t, kNumTimers> kEnds = {1500, 5432, 9876};
constexpr std::array<uint64_t, kNumTimers> kThreadIndices = {
    0, 1, 1};  // kThreadIndices[i] is the index of the thread that timer i corresponds to.

const std::array<TimerInfo, kNumTimers> kTimers = []() {
  std::array<TimerInfo, kNumTimers> timers;
  for (size_t i = 0; i < kNumTimers; i++) {
    timers[i].set_start(kStarts[i]);
    timers[i].set_end(kEnds[i]);
    timers[i].set_thread_id(kThreadIds[kThreadIndices[i]]);
    timers[i].set_function_id(kFunctionIds[0]);
  }
  return timers;
}();

const std::vector<const TimerInfo*> kTimerPointers = []() {
  std::vector<const TimerInfo*> pointers;
  std::transform(std::begin(kTimers), std::end(kTimers), std::back_inserter(pointers),
                 [](const TimerInfo& timer) { return &timer; });
  return pointers;
}();

const orbit_client_data::TimerChain kTimerChain = []() {
  orbit_client_data::TimerChain result;
  for (const auto& timer : kTimers) {
    result.emplace_back(timer);
  }
  return result;
}();

const std::vector<const orbit_client_data::TimerChain*> kTimerChains = {&kTimerChain};

const std::vector<uint64_t> kDurations = []() {
  std::vector<uint64_t> durations;
  std::transform(std::begin(kTimerPointers), std::end(kTimerPointers),
                 std::back_inserter(durations),
                 [](const TimerInfo* timer) { return timer->end() - timer->start(); });
  return durations;
}();

const std::array<ScopeStats, kNumFunctions> kScopeStats = [] {
  std::array<ScopeStats, kNumFunctions> scope_stats;
  for (size_t i = 0; i < kNumFunctions; i++) {
    scope_stats[i].set_count(kCounts[i]);
    scope_stats[i].set_total_time_ns(kTotalTimeNs[i]);
    scope_stats[i].set_min_ns(kMinNs[i]);
    scope_stats[i].set_max_ns(kMaxNs[i]);
    scope_stats[i].set_variance_ns(kStdDevNs[i] * kStdDevNs[i]);
  }
  return scope_stats;
}();

std::string GetExpectedDisplayTime(uint64_t time_ns) {
  return orbit_display_formats::GetDisplayTime(absl::Nanoseconds(time_ns));
}

std::string GetExpectedDisplayAddress(uint64_t address) { return absl::StrFormat("%#x", address); }

std::string GetExpectedDisplayCount(uint64_t count) { return absl::StrFormat("%lu", count); }

std::unique_ptr<CaptureData> GenerateTestCaptureData(
    orbit_client_data::ModuleManager* module_manager) {
  orbit_grpc_protos::CaptureStarted capture_started{};

  for (size_t i = 0; i < kNumFunctions; i++) {
    ModuleInfo module_info{};
    module_info.set_file_path(kModulePaths[i]);
    module_info.set_build_id(kBuildIds[i]);
    module_info.set_load_bias(kLoadBiases[i]);
    (void)module_manager->AddOrUpdateModules({module_info});

    orbit_grpc_protos::SymbolInfo symbol_info;
    symbol_info.set_demangled_name(kPrettyNames[i]);
    symbol_info.set_address(kAddresses[i]);
    symbol_info.set_size(kSizes[i]);

    orbit_grpc_protos::ModuleSymbols module_symbols;
    module_symbols.mutable_symbol_infos()->Add(std::move(symbol_info));

    orbit_client_data::ModuleData* module_data = module_manager->GetMutableModuleByModuleIdentifier(
        orbit_symbol_provider::ModuleIdentifier{kModulePaths[i], kBuildIds[i]});
    module_data->AddSymbols(module_symbols);

    const FunctionInfo& function = *module_data->FindFunctionByVirtualAddress(kAddresses[i], true);
    InstrumentedFunction* instrumented_function =
        capture_started.mutable_capture_options()->add_instrumented_functions();
    instrumented_function->set_file_path(function.module_path());
    instrumented_function->set_file_build_id(function.module_build_id());
    instrumented_function->set_file_offset(function.ComputeFileOffset(*module_data));
    instrumented_function->set_function_id(kFunctionIds[i]);
    instrumented_function->set_function_virtual_address(function.address());
    instrumented_function->set_function_name(kPrettyNames[i]);
  }

  auto capture_data =
      std::make_unique<CaptureData>(capture_started, std::nullopt, absl::flat_hash_set<uint64_t>{},
                                    CaptureData::DataSource::kLiveCapture);

  for (const TimerInfo* timer_info : kTimerPointers) {
    capture_data->GetThreadTrackDataProvider()->AddTimer(*timer_info);
    capture_data->UpdateScopeStats(*timer_info);
  }

  for (size_t i = 0; i < kNumFunctions; i++) {
    ScopeStats stats;
    stats.set_count(kCounts[i]);
    stats.set_total_time_ns(kTotalTimeNs[i]);
    stats.set_min_ns(kMinNs[i]);
    stats.set_max_ns(kMaxNs[i]);
    stats.set_variance_ns(kStdDevNs[i] * kStdDevNs[i]);
    capture_data->AddScopeStats(kScopeIds[i], std::move(stats));
  }

  capture_data->OnCaptureComplete();

  return capture_data;
}

class MockLiveFunctionsInterface : public orbit_data_views::LiveFunctionsInterface {
 public:
  MOCK_METHOD(void, AddIterator, (ScopeId scope_id, const FunctionInfo* function));
};

class LiveFunctionsDataViewTest : public testing::Test {
 public:
  explicit LiveFunctionsDataViewTest()
      : view_{&live_functions_, &app_}, capture_data_(GenerateTestCaptureData(&module_manager_)) {
    EXPECT_CALL(app_, GetModuleManager()).WillRepeatedly(Return(&module_manager_));
    EXPECT_CALL(app_, GetMutableModuleManager()).WillRepeatedly(Return(&module_manager_));
    EXPECT_CALL(app_, HasCaptureData).WillRepeatedly(Return(true));
    EXPECT_CALL(app_, GetCaptureData).WillRepeatedly(ReturnRef(*capture_data_));

    view_.Init();
    for (size_t i = 0; i < kNumFunctions; i++) {
      FunctionInfo function{kModulePaths[i], kBuildIds[i],    kAddresses[i],
                            /*size=*/0,      kPrettyNames[i], /*is_hotpatchable=*/false};
      functions_.insert_or_assign(kScopeIds[i], std::move(function));
    }
    view_.SetScopeStatsCollection(capture_data_->GetAllScopeStatsCollection());
  }

  void AddFunctionsByIndices(absl::Span<const size_t> indices) {
    std::set index_set(indices.begin(), indices.end());
    auto scope_stats_collection = std::make_shared<MockScopeStatsCollection>();
    std::vector<ScopeId> ids;
    absl::c_transform(index_set, std::back_inserter(ids),
                      [](const size_t index) { return ScopeId(kScopeIds[index]); });
    EXPECT_CALL(*scope_stats_collection, GetAllProvidedScopeIds).WillRepeatedly(Return(ids));
    for (size_t index : index_set) {
      EXPECT_CALL(*scope_stats_collection, GetScopeStatsOrDefault(kScopeIds[index]))
          .WillRepeatedly(ReturnRef(kScopeStats[index]));
    }
    EXPECT_CALL(*scope_stats_collection, GetSortedTimerDurationsForScopeId(kScopeIds[0]))
        .WillRepeatedly(Return(&kDurations));

    view_.SetScopeStatsCollection(std::move(scope_stats_collection));
  }

 protected:
  MockLiveFunctionsInterface live_functions_;
  orbit_data_views::MockAppInterface app_;
  orbit_data_views::LiveFunctionsDataView view_;

  orbit_client_data::ModuleManager module_manager_;
  absl::flat_hash_map<ScopeId, FunctionInfo> functions_;
  std::unique_ptr<CaptureData> capture_data_;
};

}  // namespace

TEST_F(LiveFunctionsDataViewTest, ColumnHeadersNotEmpty) {
  EXPECT_GE(view_.GetColumns().size(), 1);
  for (const auto& column : view_.GetColumns()) {
    EXPECT_FALSE(column.header.empty());
  }
}

TEST_F(LiveFunctionsDataViewTest, HasValidDefaultSortingColumn) {
  EXPECT_GE(view_.GetDefaultSortingColumn(), kColumnCount);
  EXPECT_LT(view_.GetDefaultSortingColumn(), view_.GetColumns().size());
}

TEST_F(LiveFunctionsDataViewTest, ColumnValuesAreCorrect) {
  AddFunctionsByIndices({0});

  // The selected column will be tested separately.
  EXPECT_EQ(view_.GetValue(0, kColumnName), kPrettyNames[0]);
  EXPECT_EQ(view_.GetValue(0, kColumnModule), std::filesystem::path(kModulePaths[0]).filename());
  EXPECT_EQ(view_.GetValue(0, kColumnAddress), GetExpectedDisplayAddress(kAddresses[0]));
  EXPECT_EQ(view_.GetValue(0, kColumnCount), GetExpectedDisplayCount(kCounts[0]));
  EXPECT_EQ(view_.GetValue(0, kColumnTimeTotal), GetExpectedDisplayTime(kTotalTimeNs[0]));
  EXPECT_EQ(view_.GetValue(0, kColumnTimeAvg), GetExpectedDisplayTime(kAvgTimeNs[0]));
  EXPECT_EQ(view_.GetValue(0, kColumnTimeMin), GetExpectedDisplayTime(kMinNs[0]));
  EXPECT_EQ(view_.GetValue(0, kColumnTimeMax), GetExpectedDisplayTime(kMaxNs[0]));
  EXPECT_EQ(view_.GetValue(0, kColumnStdDev), GetExpectedDisplayTime(kStdDevNs[0]));
}

TEST_F(LiveFunctionsDataViewTest, ColumnSelectedShowsRightResults) {
  bool function_selected = false;
  bool frame_track_enabled = false;
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const orbit_client_data::FunctionInfo&>()))
      .WillRepeatedly(testing::ReturnPointee(&function_selected));
  // The following code guarantees the appearance of frame track icon is determined by
  // frame_track_enable.
  EXPECT_CALL(app_, IsFrameTrackEnabled)
      .WillRepeatedly(testing::ReturnPointee(&frame_track_enabled));
  EXPECT_CALL(app_, HasFrameTrackInCaptureData)
      .WillRepeatedly(testing::ReturnPointee(&frame_track_enabled));

  AddFunctionsByIndices({0});
  EXPECT_EQ(view_.GetValue(0, kColumnSelected),
            orbit_data_views::FunctionsDataView::kDynamicallyInstrumentedFunctionTypeString);

  function_selected = true;
  EXPECT_THAT(
      view_.GetValue(0, kColumnSelected),
      testing::AllOf(
          testing::HasSubstr(
              orbit_data_views::FunctionsDataView::kDynamicallyInstrumentedFunctionTypeString),
          testing::HasSubstr(orbit_data_views::FunctionsDataView::kSelectedFunctionString),
          testing::Not(
              testing::HasSubstr(orbit_data_views::FunctionsDataView::kFrameTrackString))));

  function_selected = false;
  frame_track_enabled = true;
  EXPECT_THAT(
      view_.GetValue(0, kColumnSelected),
      testing::AllOf(
          testing::HasSubstr(
              orbit_data_views::FunctionsDataView::kDynamicallyInstrumentedFunctionTypeString),
          testing::Not(
              testing::HasSubstr(orbit_data_views::FunctionsDataView::kSelectedFunctionString)),
          testing::HasSubstr(orbit_data_views::FunctionsDataView::kFrameTrackString)));

  function_selected = true;
  EXPECT_THAT(
      view_.GetValue(0, kColumnSelected),
      testing::AllOf(
          testing::HasSubstr(
              orbit_data_views::FunctionsDataView::kDynamicallyInstrumentedFunctionTypeString),
          testing::HasSubstr(orbit_data_views::FunctionsDataView::kSelectedFunctionString),
          testing::HasSubstr(orbit_data_views::FunctionsDataView::kFrameTrackString)));
}

TEST_F(LiveFunctionsDataViewTest, ContextMenuEntriesArePresentCorrectly) {
  AddFunctionsByIndices({0, 1, 2});
  bool capture_connected;
  bool is_capturing;
  std::array<bool, kNumFunctions> functions_selected{false, true, true};
  std::array<bool, kNumFunctions> frame_track_enabled{false, false, true};
  for (size_t i = 0; i < kNumFunctions; i++) {
    if (frame_track_enabled[i]) {
      capture_data_->EnableFrameTrack(kFunctionIds[i]);
    }
  }

  auto get_index_from_function_info = [&](const FunctionInfo& function) -> std::optional<size_t> {
    for (size_t i = 0; i < kNumFunctions; i++) {
      if (kPrettyNames[i] == function.pretty_name()) return i;
    }
    return std::nullopt;
  };
  EXPECT_CALL(app_, IsCaptureConnected).WillRepeatedly(testing::ReturnPointee(&capture_connected));
  EXPECT_CALL(app_, IsCapturing).WillRepeatedly(testing::ReturnPointee(&is_capturing));
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const orbit_client_data::FunctionInfo&>()))
      .WillRepeatedly([&](const FunctionInfo& function) -> bool {
        std::optional<size_t> index = get_index_from_function_info(function);
        EXPECT_TRUE(index.has_value());
        return functions_selected.at(index.value());
      });
  EXPECT_CALL(app_, IsFrameTrackEnabled).WillRepeatedly([&](const FunctionInfo& function) -> bool {
    std::optional<size_t> index = get_index_from_function_info(function);
    EXPECT_TRUE(index.has_value());
    return frame_track_enabled.at(index.value());
  });

  auto verify_context_menu_action_availability = [&](std::vector<int> selected_indices) {
    FlattenContextMenu context_menu = FlattenContextMenuWithGroupingAndCheckOrder(
        view_.GetContextMenuWithGrouping(0, selected_indices));

    // Common actions should always be available.
    CheckSingleAction(context_menu, kMenuActionCopySelection, ContextMenuEntry::kEnabled);
    CheckSingleAction(context_menu, kMenuActionExportToCsv, ContextMenuEntry::kEnabled);
    CheckSingleAction(context_menu, kMenuActionExportEventsToCsv, ContextMenuEntry::kEnabled);

    // Source code and disassembly actions are availble if and only if capture is connected.
    ContextMenuEntry source_code_or_disassembly =
        capture_connected ? ContextMenuEntry::kEnabled : ContextMenuEntry::kDisabled;
    CheckSingleAction(context_menu, kMenuActionSourceCode, source_code_or_disassembly);
    CheckSingleAction(context_menu, kMenuActionDisassembly, source_code_or_disassembly);

    // Add iterators action is only available if some function has non-zero counts.
    int total_counts = 0;
    for (int selected_index : selected_indices) {
      total_counts += kCounts[selected_index];
    }
    ContextMenuEntry add_iterators =
        total_counts > 0 ? ContextMenuEntry::kEnabled : ContextMenuEntry::kDisabled;
    CheckSingleAction(context_menu, kMenuActionAddIterator, add_iterators);

    // Jump actions are only available for single selection with non-zero counts.
    ContextMenuEntry jump_to_direction =
        selected_indices.size() == 1 && total_counts > 0 && !is_capturing
            ? ContextMenuEntry::kEnabled
            : ContextMenuEntry::kDisabled;
    CheckSingleAction(context_menu, kMenuActionJumpToFirst, jump_to_direction);
    CheckSingleAction(context_menu, kMenuActionJumpToLast, jump_to_direction);
    CheckSingleAction(context_menu, kMenuActionJumpToMin, jump_to_direction);
    CheckSingleAction(context_menu, kMenuActionJumpToMax, jump_to_direction);

    // Hook action is available if and only if 1) capture is connected and 2) there is an unselected
    // instrumented function. Unhook action is available if and only if 1) capture is connected and
    // 2) there is a selected instrumented function.
    ContextMenuEntry select = ContextMenuEntry::kDisabled;
    ContextMenuEntry unselect = ContextMenuEntry::kDisabled;
    if (capture_connected) {
      for (size_t index : selected_indices) {
        if (functions_selected.at(index)) {
          unselect = ContextMenuEntry::kEnabled;
        } else {
          select = ContextMenuEntry::kEnabled;
        }
      }
    }
    CheckSingleAction(context_menu, kMenuActionSelect, select);
    CheckSingleAction(context_menu, kMenuActionUnselect, unselect);

    // Enable frametrack action is available if and only if there is an instrumented function with
    // frametrack not yet enabled, disable frametrack action is available if and only if there is an
    // instrumented function with frametrack enabled.
    ContextMenuEntry enable_frametrack = ContextMenuEntry::kDisabled;
    ContextMenuEntry disable_frametrack = ContextMenuEntry::kDisabled;
    for (size_t index : selected_indices) {
      if (frame_track_enabled.at(index)) {
        disable_frametrack = ContextMenuEntry::kEnabled;
      } else {
        enable_frametrack = ContextMenuEntry::kEnabled;
      }
    }
    CheckSingleAction(context_menu, kMenuActionEnableFrameTrack, enable_frametrack);
    CheckSingleAction(context_menu, kMenuActionDisableFrameTrack, disable_frametrack);
  };

  for (bool capture_connected_value : {false, true}) {
    for (bool is_capturing_value : {false, true}) {
      capture_connected = capture_connected_value;
      is_capturing = is_capturing_value;

      verify_context_menu_action_availability({0});
      verify_context_menu_action_availability({1});
      verify_context_menu_action_availability({2});
      verify_context_menu_action_availability({0, 1, 2});
    }
  }
}

TEST_F(LiveFunctionsDataViewTest, ContextMenuActionsAreInvoked) {
  bool function_selected = false;
  bool frame_track_enabled = false;
  EXPECT_CALL(app_, IsCaptureConnected).WillRepeatedly(testing::Return(true));
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const orbit_client_data::FunctionInfo&>()))
      .WillRepeatedly(testing::ReturnPointee(&function_selected));
  EXPECT_CALL(app_, IsFrameTrackEnabled)
      .WillRepeatedly(testing::ReturnPointee(&frame_track_enabled));

  AddFunctionsByIndices({0});
  FlattenContextMenu context_menu =
      FlattenContextMenuWithGroupingAndCheckOrder(view_.GetContextMenuWithGrouping(0, {0}));
  ASSERT_FALSE(context_menu.empty());

  // Copy Selection
  {
    std::string expected_clipboard = absl::StrFormat(
        "Type\tName\tCount\tTotal\tAvg\tMin\tMax\tStd Dev\tModule\tAddress\n"
        "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
        orbit_data_views::FunctionsDataView::kDynamicallyInstrumentedFunctionTypeString,
        kPrettyNames[0], GetExpectedDisplayCount(kCounts[0]),
        GetExpectedDisplayTime(kTotalTimeNs[0]), GetExpectedDisplayTime(kAvgTimeNs[0]),
        GetExpectedDisplayTime(kMinNs[0]), GetExpectedDisplayTime(kMaxNs[0]),
        GetExpectedDisplayTime(kStdDevNs[0]),
        std::filesystem::path(kModulePaths[0]).filename().string(),
        GetExpectedDisplayAddress(kAddresses[0]));
    CheckCopySelectionIsInvoked(context_menu, app_, view_, expected_clipboard);
  }

  // Export to CSV
  {
    std::string expected_contents = absl::StrFormat(
        R"("Type","Name","Count","Total","Avg","Min","Max","Std Dev","Module","Address")"
        "\r\n"
        R"("%s","%s","%s","%s","%s","%s","%s","%s","%s","%s")"
        "\r\n",
        orbit_data_views::FunctionsDataView::kDynamicallyInstrumentedFunctionTypeString,
        kPrettyNames[0], GetExpectedDisplayCount(kCounts[0]),
        GetExpectedDisplayTime(kTotalTimeNs[0]), GetExpectedDisplayTime(kAvgTimeNs[0]),
        GetExpectedDisplayTime(kMinNs[0]), GetExpectedDisplayTime(kMaxNs[0]),
        GetExpectedDisplayTime(kStdDevNs[0]),
        std::filesystem::path(kModulePaths[0]).filename().string(),
        GetExpectedDisplayAddress(kAddresses[0]));
    CheckExportToCsvIsInvoked(context_menu, app_, view_, expected_contents);
  }

  // Export events to CSV
  {
    for (size_t i = 0; i < kNumThreads; ++i) {
      capture_data_->AddOrAssignThreadName(kThreadIds[i], kThreadNames[i]);
    }

    std::string expected_contents("\"Name\",\"Thread\",\"Start\",\"End\",\"Duration (ns)\"\r\n");
    for (size_t i = 0; i < kNumTimers; ++i) {
      expected_contents += absl::StrFormat(R"("%s","%s [%lu]","%lu","%lu","%lu")"
                                           "\r\n",
                                           kPrettyNames[0], kThreadNames[kThreadIndices[i]],
                                           kThreadIds[kThreadIndices[i]], kStarts[i], kEnds[i],
                                           kEnds[i] - kStarts[i]);
    }
    CheckExportToCsvIsInvoked(context_menu, app_, view_, expected_contents,
                              kMenuActionExportEventsToCsv);
  }

  // Go to Disassembly
  {
    const int disassembly_index = GetActionIndexOnMenu(context_menu, kMenuActionDisassembly);
    EXPECT_TRUE(disassembly_index != kInvalidActionIndex);

    EXPECT_CALL(app_, Disassemble)
        .Times(1)
        .WillOnce([&](int32_t /*pid*/, const FunctionInfo& function) {
          EXPECT_EQ(function.pretty_name(), kPrettyNames[0]);
        });
    view_.OnContextMenu(std::string{kMenuActionDisassembly}, disassembly_index, {0});
  }

  // Go to Source code
  {
    const int source_code_index = GetActionIndexOnMenu(context_menu, kMenuActionDisassembly);
    EXPECT_TRUE(source_code_index != kInvalidActionIndex);

    EXPECT_CALL(app_, ShowSourceCode).Times(1).WillOnce([&](const FunctionInfo& function) {
      EXPECT_EQ(function.pretty_name(), kPrettyNames[0]);
    });
    view_.OnContextMenu(std::string{kMenuActionSourceCode}, source_code_index, {0});
  }

  // Jump to first
  {
    const int jump_to_first_index = GetActionIndexOnMenu(context_menu, kMenuActionJumpToFirst);
    EXPECT_TRUE(jump_to_first_index != kInvalidActionIndex);

    EXPECT_CALL(app_, JumpToTimerAndZoom)
        .Times(1)
        .WillOnce([](ScopeId /*scope_id*/, JumpToTimerMode selection_mode) {
          EXPECT_EQ(selection_mode, JumpToTimerMode::kFirst);
        });
    view_.OnContextMenu(std::string{kMenuActionJumpToFirst}, jump_to_first_index, {0});
  }

  // Jump to last
  {
    const int jump_to_last_index = GetActionIndexOnMenu(context_menu, kMenuActionJumpToLast);
    EXPECT_TRUE(jump_to_last_index != kInvalidActionIndex);

    EXPECT_CALL(app_, JumpToTimerAndZoom)
        .Times(1)
        .WillOnce([](ScopeId /*scope_id*/, JumpToTimerMode selection_mode) {
          EXPECT_EQ(selection_mode, JumpToTimerMode::kLast);
        });
    view_.OnContextMenu(std::string{kMenuActionJumpToLast}, jump_to_last_index, {0});
  }

  // Jump to min
  {
    const int jump_to_min_index = GetActionIndexOnMenu(context_menu, kMenuActionJumpToMin);
    EXPECT_TRUE(jump_to_min_index != kInvalidActionIndex);

    EXPECT_CALL(app_, JumpToTimerAndZoom)
        .Times(1)
        .WillOnce([](ScopeId /*scope_id*/, JumpToTimerMode selection_mode) {
          EXPECT_EQ(selection_mode, JumpToTimerMode::kMin);
        });
    view_.OnContextMenu(std::string{kMenuActionJumpToMin}, jump_to_min_index, {0});
  }

  // Jump to max
  {
    const int jump_to_max_index = GetActionIndexOnMenu(context_menu, kMenuActionJumpToMax);
    EXPECT_TRUE(jump_to_max_index != kInvalidActionIndex);

    EXPECT_CALL(app_, JumpToTimerAndZoom)
        .Times(1)
        .WillOnce([](ScopeId /*scope_id*/, JumpToTimerMode selection_mode) {
          EXPECT_EQ(selection_mode, JumpToTimerMode::kMax);
        });
    view_.OnContextMenu(std::string{kMenuActionJumpToMax}, jump_to_max_index, {0});
  }

  // Add iterator(s)
  {
    const int add_iterators_index = GetActionIndexOnMenu(context_menu, kMenuActionAddIterator);
    EXPECT_TRUE(add_iterators_index != kInvalidActionIndex);

    EXPECT_CALL(live_functions_, AddIterator)
        .Times(1)
        .WillOnce([&](ScopeId instrumented_function_id, const FunctionInfo* function) {
          EXPECT_EQ(instrumented_function_id, kScopeIds[0]);
          EXPECT_EQ(function->pretty_name(), kPrettyNames[0]);
        });
    view_.OnContextMenu(std::string{kMenuActionAddIterator}, add_iterators_index, {0});
  }

  // Hook
  {
    const auto hook_index = GetActionIndexOnMenu(context_menu, kMenuActionSelect);
    EXPECT_TRUE(hook_index != kInvalidActionIndex);

    EXPECT_CALL(app_, SelectFunction).Times(1).WillOnce([&](const FunctionInfo& function) {
      EXPECT_EQ(function.pretty_name(), kPrettyNames[0]);
    });
    view_.OnContextMenu(std::string{kMenuActionSelect}, hook_index, {0});
  }

  // Enable frame track(s)
  {
    const auto enable_frame_track_index =
        GetActionIndexOnMenu(context_menu, kMenuActionEnableFrameTrack);
    EXPECT_TRUE(enable_frame_track_index != kInvalidActionIndex);

    EXPECT_CALL(app_, SelectFunction).Times(1).WillOnce([&](const FunctionInfo& function) {
      EXPECT_EQ(function.pretty_name(), kPrettyNames[0]);
    });
    EXPECT_CALL(app_, EnableFrameTrack).Times(1);
    EXPECT_CALL(app_, AddFrameTrack).Times(1).WillOnce([&](const FunctionInfo& function) {
      EXPECT_EQ(function.pretty_name(), kPrettyNames[0]);
    });
    view_.OnContextMenu(std::string{kMenuActionEnableFrameTrack}, enable_frame_track_index, {0});
  }

  function_selected = true;
  frame_track_enabled = true;
  capture_data_->EnableFrameTrack(kFunctionIds[0]);
  context_menu =
      FlattenContextMenuWithGroupingAndCheckOrder(view_.GetContextMenuWithGrouping(0, {0}));
  ASSERT_FALSE(context_menu.empty());

  // Unhook
  {
    const auto unhook_index = GetActionIndexOnMenu(context_menu, kMenuActionUnselect);
    EXPECT_TRUE(unhook_index != kInvalidActionIndex);

    EXPECT_CALL(app_, DeselectFunction).Times(1).WillOnce([&](const FunctionInfo& function) {
      EXPECT_EQ(function.pretty_name(), kPrettyNames[0]);
    });
    EXPECT_CALL(app_, DisableFrameTrack).Times(1);
    EXPECT_CALL(app_, RemoveFrameTrack).Times(1).WillOnce([&](const FunctionInfo& function) {
      EXPECT_EQ(function.pretty_name(), kPrettyNames[0]);
    });
    view_.OnContextMenu(std::string{kMenuActionUnselect}, unhook_index, {0});
  }

  // Disable frame track(s)
  {
    const auto disable_frame_track_index =
        GetActionIndexOnMenu(context_menu, kMenuActionDisableFrameTrack);
    EXPECT_TRUE(disable_frame_track_index != kInvalidActionIndex);

    EXPECT_CALL(app_, DisableFrameTrack).Times(1).WillOnce([&](const FunctionInfo& function) {
      EXPECT_EQ(function.pretty_name(), kPrettyNames[0]);
    });
    EXPECT_CALL(app_, RemoveFrameTrack).Times(1).WillOnce([&](const FunctionInfo& function) {
      EXPECT_EQ(function.pretty_name(), kPrettyNames[0]);
    });
    view_.OnContextMenu(std::string{kMenuActionDisableFrameTrack}, disable_frame_track_index, {0});
  }
}

TEST_F(LiveFunctionsDataViewTest, FilteringShowsRightResults) {
  AddFunctionsByIndices({0, 1, 2});

  // Filtering by function display name with single token
  {
    EXPECT_CALL(app_, SetVisibleScopeIds)
        .Times(1)
        .WillOnce([&](absl::flat_hash_set<ScopeId> visible_scope_ids) {
          EXPECT_THAT(visible_scope_ids, testing::UnorderedElementsAre(kScopeIds[1], kScopeIds[2]));
        });
    view_.OnFilter("int");
    EXPECT_EQ(view_.GetNumElements(), 2);
    EXPECT_THAT((std::array{view_.GetValue(0, kColumnName), view_.GetValue(1, kColumnName)}),
                testing::UnorderedElementsAre(kPrettyNames[1], kPrettyNames[2]));
  }

  // Filtering by function display name with multiple tokens separated by " "
  {
    EXPECT_CALL(app_, SetVisibleScopeIds)
        .Times(1)
        .WillOnce([&](absl::flat_hash_set<ScopeId> visible_scope_ids) {
          EXPECT_THAT(visible_scope_ids, testing::UnorderedElementsAre(kScopeIds[1]));
        });

    view_.OnFilter("int main");
    EXPECT_EQ(view_.GetNumElements(), 1);
    EXPECT_EQ(view_.GetValue(0, kColumnName), kPrettyNames[1]);
  }

  // No matching result
  {
    EXPECT_CALL(app_, SetVisibleScopeIds)
        .Times(1)
        .WillOnce([](absl::flat_hash_set<ScopeId> visible_scope_ids) {
          EXPECT_TRUE(visible_scope_ids.empty());
        });
    view_.OnFilter("int module");
    EXPECT_EQ(view_.GetNumElements(), 0);
  }
}

TEST_F(LiveFunctionsDataViewTest, UpdateHighlightedFunctionsOnSelect) {
  AddFunctionsByIndices({0, 1, 2});

  EXPECT_CALL(app_, DeselectTimer).Times(3);
  EXPECT_CALL(app_, GetHighlightedScopeId).Times(3);

  // Single selection will hightlight the selected function
  {
    EXPECT_CALL(app_, SetHighlightedScopeId)
        .Times(1)
        .WillOnce([&](std::optional<ScopeId> highlighted_scope_id) {
          EXPECT_EQ(highlighted_scope_id, kScopeIds[2]);
        });

    view_.OnSelect({2});
  }

  // Multiple selection will hightlight the first selected function
  {
    EXPECT_CALL(app_, SetHighlightedScopeId)
        .Times(1)
        .WillOnce([&](std::optional<ScopeId> highlighted_scope_id) {
          EXPECT_EQ(highlighted_scope_id, kScopeIds[1]);
        });

    view_.OnSelect({1, 2});
  }

  // Empty selection will clear the function highlighting
  {
    EXPECT_CALL(app_, SetHighlightedScopeId)
        .Times(1)
        .WillOnce([&](std::optional<ScopeId> highlighted_scope_id) {
          EXPECT_EQ(highlighted_scope_id, std::nullopt);
        });

    view_.OnSelect({});
  }
}

TEST_F(LiveFunctionsDataViewTest, ColumnSortingShowsRightResults) {
  AddFunctionsByIndices({0, 1, 2});

  using ViewRowEntry = std::array<std::string, kNumColumns>;
  std::vector<ViewRowEntry> view_entries;
  absl::flat_hash_map<std::string, uint64_t> string_to_raw_value;
  for (const auto& [function_id, function] : functions_) {
    const ScopeStats& stats = capture_data_->GetScopeStatsOrDefault(function_id);

    ViewRowEntry entry;
    entry[kColumnName] = function.pretty_name();
    entry[kColumnModule] = std::filesystem::path(function.module_path()).filename().string();
    entry[kColumnAddress] = GetExpectedDisplayAddress(function.address());
    entry[kColumnCount] = GetExpectedDisplayCount(stats.count());
    string_to_raw_value.insert_or_assign(entry[kColumnCount], stats.count());
    entry[kColumnTimeTotal] = GetExpectedDisplayTime(stats.total_time_ns());
    string_to_raw_value.insert_or_assign(entry[kColumnTimeTotal], stats.total_time_ns());
    entry[kColumnTimeAvg] = GetExpectedDisplayTime(stats.ComputeAverageTimeNs());
    string_to_raw_value.insert_or_assign(entry[kColumnTimeAvg], stats.ComputeAverageTimeNs());
    entry[kColumnTimeMin] = GetExpectedDisplayTime(stats.min_ns());
    string_to_raw_value.insert_or_assign(entry[kColumnTimeMin], stats.min_ns());
    entry[kColumnTimeMax] = GetExpectedDisplayTime(stats.max_ns());
    string_to_raw_value.insert_or_assign(entry[kColumnTimeMax], stats.max_ns());
    entry[kColumnStdDev] = GetExpectedDisplayTime(stats.ComputeStdDevNs());
    string_to_raw_value.insert_or_assign(entry[kColumnStdDev], stats.ComputeStdDevNs());

    view_entries.push_back(entry);
  }

  auto sort_and_verify = [&](int column, orbit_data_views::DataView::SortingOrder order) {
    view_.OnSort(column, order);

    switch (column) {
      case kColumnName:
      case kColumnModule:
      case kColumnAddress:
        // Columns of name, module path and address are sort by display values (i.e., string).
        std::sort(view_entries.begin(), view_entries.end(),
                  [column, order](const ViewRowEntry& lhs, const ViewRowEntry& rhs) {
                    switch (order) {
                      case orbit_data_views::DataView::SortingOrder::kAscending:
                        return lhs[column] < rhs[column];
                      case orbit_data_views::DataView::SortingOrder::kDescending:
                        return lhs[column] > rhs[column];
                      default:
                        ORBIT_UNREACHABLE();
                    }
                  });
        break;
      case kColumnCount:
      case kColumnTimeTotal:
      case kColumnTimeAvg:
      case kColumnTimeMin:
      case kColumnTimeMax:
      case kColumnStdDev:
        // Columns of count and time statistics are sorted by raw values (i.e., uint64_t).
        std::sort(
            view_entries.begin(), view_entries.end(),
            [column, order, string_to_raw_value](const ViewRowEntry& lhs, const ViewRowEntry& rhs) {
              switch (order) {
                case orbit_data_views::DataView::SortingOrder::kAscending:
                  return string_to_raw_value.at(lhs[column]) < string_to_raw_value.at(rhs[column]);
                case orbit_data_views::DataView::SortingOrder::kDescending:
                  return string_to_raw_value.at(lhs[column]) > string_to_raw_value.at(rhs[column]);
                default:
                  ORBIT_UNREACHABLE();
              }
            });
        break;
      default:
        ORBIT_UNREACHABLE();
    }

    for (size_t index = 0; index < view_entries.size(); ++index) {
      for (int column = kColumnName; column < kNumColumns; ++column) {
        EXPECT_EQ(view_.GetValue(index, column), view_entries[index][column]);
      }
    }
  };

  for (int column = kColumnName; column < kNumColumns; ++column) {
    // Sort by ascending
    { sort_and_verify(column, orbit_data_views::DataView::SortingOrder::kAscending); }

    // Sort by descending
    { sort_and_verify(column, orbit_data_views::DataView::SortingOrder::kDescending); }
  }
}

TEST_F(LiveFunctionsDataViewTest, OnDataChangeResetsHistogram) {
  EXPECT_CALL(app_, ShowHistogram(nullptr, "", std::optional<ScopeId>{})).Times(1);

  view_.OnDataChanged();
}

TEST_F(LiveFunctionsDataViewTest, OnRefreshWithNoIndicesResetsHistogram) {
  EXPECT_CALL(app_, ShowHistogram(nullptr, "", std::optional<ScopeId>{})).Times(2);

  view_.OnRefresh({}, RefreshMode::kOnFilter);
  view_.OnRefresh({}, RefreshMode::kOther);
}

TEST_F(LiveFunctionsDataViewTest, HistogramIsProperlyUpdated) {
  EXPECT_CALL(app_, ProvideScopeId).WillRepeatedly(Invoke([&](const TimerInfo& timer) {
    return timer.function_id();
  }));

  AddFunctionsByIndices({0});

  EXPECT_CALL(app_, ShowHistogram(testing::Pointee(kDurations), kPrettyNames[0],
                                  std::optional<ScopeId>(kScopeIds[0])))
      .Times(3);

  view_.OnRefresh({0}, RefreshMode::kOnFilter);
  view_.OnRefresh({0}, RefreshMode::kOther);
  view_.UpdateHistogramWithScopeIds({kScopeIds[0]});
}

TEST_F(LiveFunctionsDataViewTest,
       RemoveHistogramWhenUpdatedWithIdOfNonDynamicallyInstrumentedFunction) {
  EXPECT_CALL(app_, ShowHistogram(nullptr, "", std::optional<ScopeId>{})).Times(1);

  view_.UpdateHistogramWithScopeIds({kNonDynamicallyInstrumentedFunctionId});
}

TEST_F(LiveFunctionsDataViewTest, LiveTabUsesScopeStatsCollection) {
  auto scope_stats_collection = std::make_shared<orbit_client_data::MockScopeStatsCollection>();
  EXPECT_CALL(*scope_stats_collection, GetAllProvidedScopeIds)
      .Times(2)
      .WillRepeatedly(Return(std::vector<ScopeId>{}));
  view_.SetScopeStatsCollection(scope_stats_collection);
  EXPECT_EQ(view_.GetRowFromScopeId(kScopeIds[0]), std::nullopt);
}

TEST_F(LiveFunctionsDataViewTest, OnDataChangedUsesScopeStatsCollectionUpdates) {
  auto scope_stats_collection = std::make_shared<orbit_client_data::MockScopeStatsCollection>();
  view_.SetScopeStatsCollection(scope_stats_collection);
  EXPECT_EQ(view_.GetRowFromScopeId(kScopeIds[0]), std::nullopt);
  EXPECT_CALL(*scope_stats_collection, GetAllProvidedScopeIds)
      .Times(2)
      .WillRepeatedly(Return(std::vector<ScopeId>{kScopeIds[0]}));
  view_.OnDataChanged();
  EXPECT_EQ(view_.GetRowFromScopeId(kScopeIds[0]), 0);
}