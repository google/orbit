// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>
#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ClientData/CallstackData.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/CallstackType.h"
#include "ClientData/CaptureData.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "ClientData/ProcessData.h"
#include "DataViewTestUtils.h"
#include "DataViews/CallstackDataView.h"
#include "DataViews/DataView.h"
#include "DataViews/SamplingReportDataView.h"
#include "DataViews/SamplingReportInterface.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "MockAppInterface.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "Statistics/BinomialConfidenceInterval.h"
#include "SymbolProvider/ModuleIdentifier.h"

using orbit_client_data::CaptureData;
using orbit_client_data::FunctionInfo;
using orbit_client_data::ModuleData;
using orbit_client_data::ModuleManager;
using orbit_client_data::ProcessData;
using orbit_client_data::SampledFunction;
using orbit_symbol_provider::ModuleIdentifier;

using orbit_data_views::CheckCopySelectionIsInvoked;
using orbit_data_views::CheckExportToCsvIsInvoked;
using orbit_data_views::CheckSingleAction;
using orbit_data_views::ContextMenuEntry;
using orbit_data_views::FlattenContextMenu;
using orbit_data_views::FlattenContextMenuWithGroupingAndCheckOrder;
using orbit_data_views::GetActionIndexOnMenu;
using orbit_data_views::kInvalidActionIndex;
using orbit_data_views::kMenuActionCopySelection;
using orbit_data_views::kMenuActionDisassembly;
using orbit_data_views::kMenuActionExportToCsv;
using orbit_data_views::kMenuActionLoadSymbols;
using orbit_data_views::kMenuActionSelect;
using orbit_data_views::kMenuActionSourceCode;
using orbit_data_views::kMenuActionUnselect;

using orbit_grpc_protos::ModuleInfo;

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace {

class MockBinomialConfidenceIntervalEstimator
    : public orbit_statistics::BinomialConfidenceIntervalEstimator {
 public:
  MOCK_METHOD(orbit_statistics::BinomialConfidenceInterval, Estimate,
              (float ratio, uint32_t trials), (const override));
};

constexpr float kConfidenceIntervalLeftSectionLength = 0.15f;
constexpr float kConfidenceIntervalRightSectionLength = 0.2f;
constexpr float kConfidenceIntervalLongerSectionLength =
    std::max(kConfidenceIntervalLeftSectionLength, kConfidenceIntervalRightSectionLength);

constexpr int kColumnSelected = 0;
constexpr int kColumnFunctionName = 1;
constexpr int kColumnInclusive = 2;
constexpr int kColumnExclusive = 3;
constexpr int kColumnModuleName = 4;
constexpr int kColumnAddress = 5;
constexpr int kColumnUnwindErrors = 6;
constexpr int kNumColumns = 7;

constexpr size_t kNumFunctions = 4;

// Used for setting up FunctionInfo
const std::array<std::string, kNumFunctions> kFunctionPrettyNames{"void foo()", "main(int, char**)",
                                                                  "ffind(int)", "bar(const char*)"};
constexpr std::array<uint64_t, kNumFunctions> kFunctionAddresses{0x5100, 0x7250, 0x6700, 0x4450};
constexpr std::array<uint64_t, kNumFunctions> kFunctionSizes{0x50, 0x70, 0x60, 0x40};

// Used for setting up ModuleData
const std::array<std::string, kNumFunctions> kModuleNames{"foo_module", "some_module",
                                                          "ffind_module", "bar_module"};
const std::array<std::string, kNumFunctions> kModulePaths{
    "/path/to/foomodule", "/path/to/somemodule", "/path/to/ffindmodule", "/path/to/barmodule"};
const std::array<std::string, kNumFunctions> kModuleBuildIds{"build_id_0", "build_id_1",
                                                             "build_id_2", "build_id_3"};
constexpr std::array<uint64_t, kNumFunctions> kModuleStartAddresses{0x3000, 0x9000, 0x7000, 0x5000};
constexpr std::array<uint64_t, kNumFunctions> kModuleEndAddresses{0x3900, 0x9500, 0x8900, 0x5500};
constexpr std::array<uint64_t, kNumFunctions> kModuleExecutableSegmentOffsets{0x123, 0x234, 0x135,
                                                                              0x246};
constexpr std::array<uint64_t, kNumFunctions> kModuleLoadBiases{0x5000, 0x7000, 0x6000, 0x4000};
constexpr std::array<ModuleData::SymbolCompleteness, kNumFunctions> kModuleSymbolCompleteness{
    ModuleData::SymbolCompleteness::kDebugSymbols,
    ModuleData::SymbolCompleteness::kDynamicLinkingAndUnwindInfo,
    ModuleData::SymbolCompleteness::kNoSymbols, ModuleData::SymbolCompleteness::kNoSymbols};

// Used for setting up SampledFunction
constexpr std::array<uint64_t, kNumFunctions> kSampledAbsoluteAddresses{0x3140, 0x9260, 0x7750,
                                                                        0x4900};
constexpr std::array<uint32_t, kNumFunctions> kSampledExclusives{3, 6, 1, 0};
constexpr std::array<float, kNumFunctions> kSampledExclusivePercents{0.08f, 0.16f, 0.03f, 0.0f};
constexpr std::array<uint32_t, kNumFunctions> kSampledInclusives{3, 6, 1, 593};
constexpr std::array<float, kNumFunctions> kSampledInclusivePercents{0.08f, 0.16f, 0.03f, 16.0f};
constexpr std::array<uint32_t, kNumFunctions> kSampledUnwindErrors{30, 8, 2, 0};
constexpr std::array<float, kNumFunctions> kSampledUnwindErrorPercents{0.8f, 0.2f, 0.06f, 0.0f};
constexpr uint32_t kStackEventsCount = 3700;

constexpr size_t kCallstackInfoNum = 3;
const std::vector<orbit_client_data::CallstackInfo> kCallstackInfos = [] {
  std::array<std::vector<uint64_t>, kCallstackInfoNum> array_of_vectors_of_frames = {
      std::vector<uint64_t>{kSampledAbsoluteAddresses[0], kSampledAbsoluteAddresses[2],
                            kSampledAbsoluteAddresses[1]},
      std::vector<uint64_t>{kSampledAbsoluteAddresses[2], kSampledAbsoluteAddresses[0],
                            kSampledAbsoluteAddresses[2]},
      std::vector<uint64_t>{kSampledAbsoluteAddresses[2], kSampledAbsoluteAddresses[1]}};
  std::vector<orbit_client_data::CallstackInfo> result;
  std::transform(
      std::begin(array_of_vectors_of_frames), std::end(array_of_vectors_of_frames),
      std::back_inserter(result),
      [](absl::Span<uint64_t const> frames) -> orbit_client_data::CallstackInfo {
        return {{frames.begin(), frames.end()}, orbit_client_data::CallstackType::kComplete};
      });
  return result;
}();
constexpr std::array<uint64_t, kCallstackInfoNum> kTimestamps = {123456, 456789, 789456};
constexpr std::array<uint32_t, kCallstackInfoNum> kTids = {321, 321, 987};
const std::array<std::string, kCallstackInfoNum> kThreadNames = {"Thread 321", "Thread 321",
                                                                 "Thread 987"};
constexpr std::array<uint64_t, kCallstackInfoNum> kCallstackIds = {123, 345, 567};
const absl::flat_hash_set<uint64_t> kSelectedCallstackIds = {kCallstackIds[1], kCallstackIds[2]};

const std::unique_ptr<const orbit_client_data::CallstackData> kCallstackData = [] {
  auto result = std::make_unique<orbit_client_data::CallstackData>();
  for (size_t i = 0; i < kCallstackInfoNum; ++i) {
    result->AddUniqueCallstack(kCallstackIds[i], kCallstackInfos[i]);
    result->AddCallstackEvent({kTimestamps[i], kCallstackIds[i], kTids[i]});
  }
  return result;
}();

[[nodiscard]] std::string GetPrettyName(uint64_t address) {
  const size_t index = std::distance(std::begin(kSampledAbsoluteAddresses),
                                     std::find(std::begin(kSampledAbsoluteAddresses),
                                               std::end(kSampledAbsoluteAddresses), address));
  return (kModuleSymbolCompleteness[index] > ModuleData::SymbolCompleteness::kNoSymbols)
             ? kFunctionPrettyNames[index]
             : "???";
}

[[nodiscard]] std::string BuildExpectedExportEventsToCsvString(const std::vector<size_t>& indices) {
  std::string result =
      "\"Thread\",\"Timestamp (ns)\",\"Names leaf/foo/main\",\"Addresses "
      "leaf_addr/foo_addr/main_addr\"";
  result.append(orbit_data_views::kLineSeparator);
  for (size_t index : indices) {
    result.append(orbit_data_views::FormatValueForCsv(
        absl::StrFormat("%s [%u]", kThreadNames[index], kTids[index])));
    result.append(orbit_data_views::kFieldSeparator);

    result.append(orbit_data_views::FormatValueForCsv(absl::StrFormat("%u", kTimestamps[index])));
    result.append(orbit_data_views::kFieldSeparator);

    const std::vector<uint64_t>& frames = kCallstackInfos[index].frames();
    std::vector<std::string> names;
    std::vector<std::string> address_strs;

    std::transform(std::begin(frames), std::end(frames), std::back_inserter(names),
                   [](uint64_t address) { return GetPrettyName(address); });
    std::transform(std::begin(frames), std::end(frames), std::back_inserter(address_strs),
                   [](uint64_t address) { return absl::StrFormat("%#llx", address); });

    constexpr std::string_view kFramesSeparator = "/";
    result.append(orbit_data_views::FormatValueForCsv(absl::StrJoin(names, kFramesSeparator)));
    result.append(orbit_data_views::kFieldSeparator);

    result.append(
        orbit_data_views::FormatValueForCsv(absl::StrJoin(address_strs, kFramesSeparator)));
    result.append(orbit_data_views::kLineSeparator);
  }
  return result;
}

std::unique_ptr<CaptureData> GenerateTestCaptureData(
    orbit_client_data::ModuleManager* module_manager) {
  std::vector<ModuleInfo> modules;

  for (size_t i = 0; i < kNumFunctions; i++) {
    ModuleInfo module_info{};
    module_info.set_name(kModuleNames[i]);
    module_info.set_file_path(kModulePaths[i]);
    module_info.set_build_id(kModuleBuildIds[i]);
    module_info.set_address_start(kModuleStartAddresses[i]);
    module_info.set_address_end(kModuleEndAddresses[i]);
    module_info.set_executable_segment_offset(kModuleExecutableSegmentOffsets[i]);
    module_info.set_load_bias(kModuleLoadBiases[i]);
    (void)module_manager->AddOrUpdateModules({module_info});

    modules.push_back(module_info);

    if (kModuleSymbolCompleteness[i] > ModuleData::SymbolCompleteness::kNoSymbols) {
      orbit_grpc_protos::SymbolInfo symbol_info;
      symbol_info.set_demangled_name(kFunctionPrettyNames[i]);
      symbol_info.set_address(kFunctionAddresses[i]);
      symbol_info.set_size(kFunctionSizes[i]);

      orbit_grpc_protos::ModuleSymbols module_symbols;
      module_symbols.mutable_symbol_infos()->Add(std::move(symbol_info));

      ModuleData* module_data = module_manager->GetMutableModuleByModuleIdentifier(
          ModuleIdentifier{kModulePaths[i], kModuleBuildIds[i]});
      switch (kModuleSymbolCompleteness[i]) {
        case ModuleData::SymbolCompleteness::kNoSymbols:
          ORBIT_UNREACHABLE();
        case ModuleData::SymbolCompleteness::kDynamicLinkingAndUnwindInfo:
          module_data->AddFallbackSymbols(module_symbols);
          break;
        case ModuleData::SymbolCompleteness::kDebugSymbols:
          module_data->AddSymbols(module_symbols);
          break;
      }
    }
  }

  constexpr int32_t kProcessId = 42;
  const std::string executable_path = "/path/to/text.exe";
  orbit_grpc_protos::CaptureStarted capture_started{};
  capture_started.set_process_id(kProcessId);
  capture_started.set_executable_path(executable_path);

  auto capture_data =
      std::make_unique<CaptureData>(capture_started, std::nullopt, absl::flat_hash_set<uint64_t>{},
                                    CaptureData::DataSource::kLiveCapture);
  ProcessData* process = capture_data.get()->mutable_process();
  process->UpdateModuleInfos(modules);

  for (size_t i = 0; i < kCallstackInfoNum; ++i) {
    capture_data->AddOrAssignThreadName(kTids[i], kThreadNames[i]);
  }

  return capture_data;
}

std::string GetExpectedDisplayAddressByIndex(size_t index) {
  return absl::StrFormat("%#llx", kSampledAbsoluteAddresses[index]);
}

std::string GetExpectedDisplayFunctionNameByIndex(size_t index, const ModuleManager& module_manager,
                                                  const CaptureData& capture_data) {
  return orbit_client_data::GetFunctionNameByAddress(module_manager, capture_data,
                                                     kSampledAbsoluteAddresses[index]);
}

std::string GetExpectedDisplayModuleNameByIndex(size_t index, const ModuleManager& module_manager,
                                                const CaptureData& capture_data) {
  std::string module_path = orbit_client_data::GetModulePathByAddress(
      module_manager, capture_data, kSampledAbsoluteAddresses[index]);
  return std::filesystem::path(module_path).filename().string();
}

std::string GetExpectedDisplayExclusiveByIndex(size_t index, bool for_copy = false) {
  if (for_copy) {
    return absl::StrFormat("%.2f%%", kSampledExclusivePercents[index]);
  }
  return absl::StrFormat("%.1f ±%.1f", kSampledExclusivePercents[index],
                         kConfidenceIntervalLongerSectionLength * 100.0f);
}

std::string GetExpectedDisplayInclusiveByIndex(size_t index, bool for_copy = false) {
  if (for_copy) {
    return absl::StrFormat("%.2f%%", kSampledInclusivePercents[index]);
  }
  return absl::StrFormat("%.1f ±%.1f", kSampledInclusivePercents[index],
                         kConfidenceIntervalLongerSectionLength * 100.0f);
}

std::string GetExpectedToolTipByIndex(size_t index, int column, const ModuleManager& module_manager,
                                      const CaptureData& capture_data) {
  if (column == kColumnSelected) {
    return "";
  }
  if (column == kColumnFunctionName) {
    return GetExpectedDisplayFunctionNameByIndex(index, module_manager, capture_data);
  }
  if (column == kColumnInclusive || column == kColumnExclusive) {
    const uint32_t raw_count =
        (column == kColumnInclusive) ? kSampledInclusives[index] : kSampledExclusives[index];
    const float percentage = (column == kColumnInclusive) ? kSampledInclusivePercents[index]
                                                          : kSampledExclusivePercents[index];
    const std::string count_type = (column == kColumnInclusive) ? "inclusive" : "exclusive";
    const std::string at_the_top_or_encountered =
        (column == kColumnInclusive) ? "encountered" : "at the top of the callstack";

    return absl::StrFormat(
        "The function \"%s\"\n"
        "was %s %u times (%s count)\n"
        "in a total of %u stack samples.\n"
        "This makes up for %.2f%% of samples.\n\n"
        "The 95%% confidence interval for the true percentage is\n"
        "(%.2f%%, %.2f%%).",
        kFunctionPrettyNames[index], at_the_top_or_encountered, raw_count, count_type,
        kStackEventsCount, percentage, percentage - kConfidenceIntervalLeftSectionLength * 100.0f,
        percentage + kConfidenceIntervalRightSectionLength * 100.0f);
  }
  if (column == kColumnModuleName) {
    return GetExpectedDisplayModuleNameByIndex(index, module_manager, capture_data);
  }
  if (column == kColumnAddress) {
    return GetExpectedDisplayAddressByIndex(index);
  }
  if (column == kColumnUnwindErrors) {
    const float percentage = kSampledUnwindErrorPercents[index];
    return absl::StrFormat(
        "%u samples with the function \"%s\"\n"
        "at the top of the stack could not be unwound\n"
        "in a total of %u stack samples.\n"
        "This makes up for %.2f%% of samples.\n\n"
        "The 95%% confidence interval for the true percentage is\n"
        "(%.2f%%, %.2f%%).",
        kSampledUnwindErrors[index], kFunctionPrettyNames[index], kStackEventsCount, percentage,
        percentage - kConfidenceIntervalLeftSectionLength * 100.0f,
        percentage + kConfidenceIntervalRightSectionLength * 100.0f);
  }
  return "";
}

std::string GetExpectedDisplayUnwindErrorsByIndex(size_t index, bool for_copy = false) {
  if (kSampledUnwindErrors[index] <= 0) return "";

  if (for_copy) {
    return absl::StrFormat("%.2f%%", kSampledUnwindErrorPercents[index]);
  }
  return absl::StrFormat("%.1f ±%.1f", kSampledUnwindErrorPercents[index],
                         kConfidenceIntervalLongerSectionLength * 100.0f);
}

class MockSamplingReportInterface : public orbit_data_views::SamplingReportInterface {
 public:
  MOCK_METHOD(void, SetCallstackDataView, (orbit_data_views::CallstackDataView*));
  MOCK_METHOD(void, OnSelectAddresses,
              (const absl::flat_hash_set<uint64_t>&, orbit_client_data::ThreadID));
  MOCK_METHOD(const orbit_client_data::CallstackData&, GetCallstackData, (), (const));
  MOCK_METHOD(std::optional<absl::flat_hash_set<uint64_t>>, GetSelectedCallstackIds, (), (const));
};

class SamplingReportDataViewTest : public testing::Test {
 public:
  explicit SamplingReportDataViewTest()
      : view_{&app_}, capture_data_(GenerateTestCaptureData(&module_manager_)) {
    EXPECT_CALL(app_, GetModuleManager()).WillRepeatedly(Return(&module_manager_));
    EXPECT_CALL(app_, GetMutableModuleManager()).WillRepeatedly(Return(&module_manager_));
    EXPECT_CALL(app_, GetConfidenceIntervalEstimator())
        .WillRepeatedly(ReturnRef(confidence_interval_estimator_));

    EXPECT_CALL(confidence_interval_estimator_, Estimate)
        .WillRepeatedly(testing::Invoke([](float ratio, uint32_t /*trials*/) {
          orbit_statistics::BinomialConfidenceInterval confidence_interval{
              ratio - kConfidenceIntervalLeftSectionLength,
              ratio + kConfidenceIntervalRightSectionLength};
          return confidence_interval;
        }));

    view_.Init();
    for (size_t i = 0; i < kNumFunctions; i++) {
      SampledFunction sampled_function;
      sampled_function.absolute_address = kSampledAbsoluteAddresses[i];
      sampled_function.name = orbit_client_data::GetFunctionNameByAddress(
          module_manager_, *capture_data_, kSampledAbsoluteAddresses[i]);
      sampled_function.module_path = orbit_client_data::GetModulePathByAddress(
          module_manager_, *capture_data_, kSampledAbsoluteAddresses[i]);
      sampled_function.exclusive = kSampledExclusives[i];
      sampled_function.exclusive_percent = kSampledExclusivePercents[i];
      sampled_function.inclusive = kSampledInclusives[i];
      sampled_function.inclusive_percent = kSampledInclusivePercents[i];
      sampled_function.unwind_errors = kSampledUnwindErrors[i];
      sampled_function.unwind_errors_percent = kSampledUnwindErrorPercents[i];
      sampled_function.function = nullptr;
      sampled_functions_.push_back(std::move(sampled_function));
    }

    view_.SetSamplingReport(&sampling_report_);
  }

  void AddFunctionsByIndices(absl::Span<const size_t> indices) {
    std::vector<SampledFunction> functions_to_add;
    for (size_t index : indices) {
      ORBIT_CHECK(index < kNumFunctions);
      functions_to_add.push_back(sampled_functions_[index]);
    }

    view_.SetSampledFunctions(functions_to_add);
  }

 protected:
  MockSamplingReportInterface sampling_report_;
  MockBinomialConfidenceIntervalEstimator confidence_interval_estimator_;
  orbit_data_views::MockAppInterface app_;
  orbit_data_views::SamplingReportDataView view_;

  orbit_client_data::ModuleManager module_manager_;
  std::unique_ptr<CaptureData> capture_data_;
  std::vector<SampledFunction> sampled_functions_;
};

}  // namespace

TEST_F(SamplingReportDataViewTest, ColumnHeadersNotEmpty) {
  EXPECT_GE(view_.GetColumns().size(), 1);
  for (const auto& column : view_.GetColumns()) {
    EXPECT_FALSE(column.header.empty());
  }
}

TEST_F(SamplingReportDataViewTest, HasValidDefaultSortingColumn) {
  EXPECT_GE(view_.GetDefaultSortingColumn(), kColumnInclusive);
  EXPECT_LT(view_.GetDefaultSortingColumn(), view_.GetColumns().size());
}

TEST_F(SamplingReportDataViewTest, ToolTipMessageIsCorrect) {
  AddFunctionsByIndices({0});
  view_.SetStackEventsCount(kStackEventsCount);
  for (size_t column = 0; column < kNumColumns; ++column) {
    EXPECT_EQ(view_.GetToolTip(0, column),
              GetExpectedToolTipByIndex(0, column, module_manager_, *capture_data_));
  }
}

TEST_F(SamplingReportDataViewTest, ColumnValuesAreCorrect) {
  AddFunctionsByIndices({0});

  // The selected column will be tested separately.
  EXPECT_EQ(view_.GetValue(0, kColumnAddress), GetExpectedDisplayAddressByIndex(0));
  EXPECT_EQ(view_.GetValue(0, kColumnFunctionName),
            GetExpectedDisplayFunctionNameByIndex(0, module_manager_, *capture_data_));
  EXPECT_EQ(view_.GetValue(0, kColumnModuleName),
            GetExpectedDisplayModuleNameByIndex(0, module_manager_, *capture_data_));
  EXPECT_EQ(view_.GetValue(0, kColumnExclusive), GetExpectedDisplayExclusiveByIndex(0));
  EXPECT_EQ(view_.GetValue(0, kColumnInclusive), GetExpectedDisplayInclusiveByIndex(0));
  EXPECT_EQ(view_.GetValue(0, kColumnUnwindErrors), GetExpectedDisplayUnwindErrorsByIndex(0));
}

TEST_F(SamplingReportDataViewTest, ColumnSelectedShowsRightResults) {
  bool function_selected = false;
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const SampledFunction&>()))
      .WillRepeatedly(testing::ReturnPointee(&function_selected));

  AddFunctionsByIndices({0});
  EXPECT_EQ(view_.GetValue(0, kColumnSelected), "");

  function_selected = true;
  EXPECT_EQ(view_.GetValue(0, kColumnSelected), "H");
}

TEST_F(SamplingReportDataViewTest, ContextMenuEntriesArePresentCorrectly) {
  EXPECT_CALL(app_, GetCaptureData).WillRepeatedly(testing::ReturnRef(*capture_data_));
  EXPECT_CALL(app_, GetMutableModuleByModuleIdentifier)
      .WillRepeatedly([&](const ModuleIdentifier& module_id) -> ModuleData* {
        return module_manager_.GetMutableModuleByModuleIdentifier(module_id);
      });

  bool capture_connected;
  EXPECT_CALL(app_, IsCaptureConnected).WillRepeatedly(testing::ReturnPointee(&capture_connected));

  std::array<bool, kNumFunctions> functions_selected{true, false, false, true};
  auto get_index_from_function_info = [&](const FunctionInfo& function) -> std::optional<size_t> {
    for (size_t i = 0; i < kNumFunctions; i++) {
      if (kFunctionPrettyNames[i] == function.pretty_name()) return i;
    }
    return std::nullopt;
  };
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const orbit_client_data::FunctionInfo&>()))
      .WillRepeatedly([&](const FunctionInfo& function) -> bool {
        std::optional<size_t> index = get_index_from_function_info(function);
        EXPECT_TRUE(index.has_value());
        return functions_selected.at(index.value());
      });

  auto get_context_menu_from_selected_indices =
      [&](absl::Span<const int> selected_indices) -> FlattenContextMenu {
    std::vector<int> selected_rows;
    for (int index : selected_indices) {
      for (int row = 0, row_counts = view_.GetNumElements(); row < row_counts; row++) {
        if (view_.GetValue(row, kColumnAddress) == GetExpectedDisplayAddressByIndex(index)) {
          selected_rows.push_back(row);
          break;
        }
      }
    }
    return FlattenContextMenuWithGroupingAndCheckOrder(
        view_.GetContextMenuWithGrouping(0, selected_rows));
  };

  auto verify_context_menu_action_availability = [&](absl::Span<const int> selected_indices) {
    FlattenContextMenu context_menu = get_context_menu_from_selected_indices(selected_indices);

    // Common actions should always be available.
    CheckSingleAction(context_menu, kMenuActionCopySelection, ContextMenuEntry::kEnabled);
    CheckSingleAction(context_menu, kMenuActionExportToCsv, ContextMenuEntry::kEnabled);

    // Find indices that SamplingReportDataView::GetFunctionInfoFromRow can find matching functions.
    std::vector<int> selected_indices_with_matching_function;
    for (int index : selected_indices) {
      // SamplingReportDataView::GetFunctionInfoFromRow cannot find matching FunctionInfo for
      // sampled function 2 & 3 as:
      // * sampled function 2's corresponding module is not loaded yet
      // * sampled function 3's absolute address does not match any module and function.
      if (index < 2) selected_indices_with_matching_function.push_back(index);
    }

    // Source code and disassembly actions are availble if and only if 1) capture is connected and
    // 2) exist an index that GetFunctionInfoFromRow can find matching function.
    ContextMenuEntry source_code_or_disassembly =
        capture_connected && !selected_indices_with_matching_function.empty()
            ? ContextMenuEntry::kEnabled
            : ContextMenuEntry::kDisabled;
    CheckSingleAction(context_menu, kMenuActionSourceCode, source_code_or_disassembly);
    CheckSingleAction(context_menu, kMenuActionDisassembly, source_code_or_disassembly);

    // Hook action is available if and only if 1) capture is connected and 2) exist an index so that
    // the function returned by GetFunctionInfoFromRow(index) is selected.
    // Unhook action is available if and only if 1) capture is connected and 2) exist an index so
    // that the function returned by GetFunctionInfoFromRow(index) is selected.
    ContextMenuEntry select = ContextMenuEntry::kDisabled;
    ContextMenuEntry unselect = ContextMenuEntry::kDisabled;
    if (capture_connected) {
      for (size_t index : selected_indices_with_matching_function) {
        if (!functions_selected[index]) {
          select = ContextMenuEntry::kEnabled;
        } else {
          unselect = ContextMenuEntry::kEnabled;
        }
      }
    }
    CheckSingleAction(context_menu, kMenuActionSelect, select);
    CheckSingleAction(context_menu, kMenuActionUnselect, unselect);

    // Find indices that SamplingReportDataView::GetModuleIdentifierFromRow can find matching
    // ModuleIdentifier
    std::vector<int> selected_indices_with_matching_module;
    for (int index : selected_indices) {
      // Sampled function 3's absolute address does not match any module
      if (index != 3) selected_indices_with_matching_module.push_back(index);
    }

    // Load symbols action is available if and only if: for all ModuleIdentifiers returned by
    // SamplingReportDataView::GetModuleIdentifierFromRow, there is corresponding module that is not
    // loaded yet.
    ContextMenuEntry load_symbols = ContextMenuEntry::kDisabled;
    for (int index : selected_indices_with_matching_module) {
      if (kModuleSymbolCompleteness[index] < ModuleData::SymbolCompleteness::kDebugSymbols) {
        load_symbols = ContextMenuEntry::kEnabled;
        break;
      }
    }
    CheckSingleAction(context_menu, kMenuActionLoadSymbols, load_symbols);
  };

  AddFunctionsByIndices({0, 1, 2, 3});
  capture_connected = false;
  verify_context_menu_action_availability({0});
  verify_context_menu_action_availability({1});
  verify_context_menu_action_availability({2});
  verify_context_menu_action_availability({3});
  verify_context_menu_action_availability({0, 1, 2, 3});

  capture_connected = true;
  verify_context_menu_action_availability({0});
  verify_context_menu_action_availability({1});
  verify_context_menu_action_availability({2});
  verify_context_menu_action_availability({3});
  verify_context_menu_action_availability({0, 1, 2, 3});
}

TEST_F(SamplingReportDataViewTest, ContextMenuActionsAreInvoked) {
  bool function_selected = false;

  EXPECT_CALL(app_, GetCaptureData).WillRepeatedly(testing::ReturnRef(*capture_data_));
  EXPECT_CALL(app_, IsCaptureConnected).WillRepeatedly(testing::Return(true));
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const orbit_client_data::FunctionInfo&>()))
      .WillRepeatedly(testing::ReturnPointee(&function_selected));
  EXPECT_CALL(app_, GetMutableModuleByModuleIdentifier)
      .WillRepeatedly([&](const ModuleIdentifier& module_id) -> ModuleData* {
        return module_manager_.GetMutableModuleByModuleIdentifier(module_id);
      });

  AddFunctionsByIndices({0});
  FlattenContextMenu context_menu =
      FlattenContextMenuWithGroupingAndCheckOrder(view_.GetContextMenuWithGrouping(0, {0}));
  ASSERT_FALSE(context_menu.empty());

  // Copy Selection
  {
    std::string expected_clipboard = absl::StrFormat(
        "Hooked\tName\tInclusive, %%\tExclusive, %%\tModule\tAddress\tUnwind errors, %%\n"
        "\t%s\t%s\t%s\t%s\t%s\t%s\n",
        GetExpectedDisplayFunctionNameByIndex(0, module_manager_, *capture_data_),
        GetExpectedDisplayInclusiveByIndex(0, true), GetExpectedDisplayExclusiveByIndex(0, true),
        GetExpectedDisplayModuleNameByIndex(0, module_manager_, *capture_data_),
        GetExpectedDisplayAddressByIndex(0), GetExpectedDisplayUnwindErrorsByIndex(0, true));
    CheckCopySelectionIsInvoked(context_menu, app_, view_, expected_clipboard);
  }

  // Export to CSV
  {
    std::string expected_contents = absl::StrFormat(
        R"("Hooked","Name","Inclusive, %%","Exclusive, %%","Module","Address","Unwind errors, %%")"
        "\r\n"
        R"("","%s","%s","%s","%s","%s","%s")"
        "\r\n",
        GetExpectedDisplayFunctionNameByIndex(0, module_manager_, *capture_data_),
        GetExpectedDisplayInclusiveByIndex(0, true), GetExpectedDisplayExclusiveByIndex(0, true),
        GetExpectedDisplayModuleNameByIndex(0, module_manager_, *capture_data_),
        GetExpectedDisplayAddressByIndex(0), GetExpectedDisplayUnwindErrorsByIndex(0, true));
    CheckExportToCsvIsInvoked(context_menu, app_, view_, expected_contents);
  }

  // Export Callstack Events to CSV
  {
    EXPECT_CALL(sampling_report_, GetCallstackData).WillRepeatedly(ReturnRef(*kCallstackData));
    EXPECT_CALL(sampling_report_, GetSelectedCallstackIds)
        .Times(4)
        .WillOnce(Return(std::nullopt))
        .WillOnce(Return(std::nullopt))
        .WillOnce(Return(kSelectedCallstackIds))
        .WillOnce(Return(kSelectedCallstackIds));

    // Test the case of all threads, no active selection
    view_.SetThreadID(orbit_base::kAllProcessThreadsTid);
    CheckExportToCsvIsInvoked(context_menu, app_, view_,
                              BuildExpectedExportEventsToCsvString({0, 1, 2}),
                              orbit_data_views::kMenuActionExportEventsToCsv);

    // Test the case of one thread, no active selection
    view_.SetThreadID(kTids[0]);
    CheckExportToCsvIsInvoked(context_menu, app_, view_,
                              BuildExpectedExportEventsToCsvString({0, 1}),
                              orbit_data_views::kMenuActionExportEventsToCsv);

    // Test the case of all threads, with an active selection
    view_.SetThreadID(orbit_base::kAllProcessThreadsTid);
    CheckExportToCsvIsInvoked(context_menu, app_, view_,
                              BuildExpectedExportEventsToCsvString({1, 2}),
                              orbit_data_views::kMenuActionExportEventsToCsv);

    // Test the case of one thread, with an active selection
    view_.SetThreadID(kTids[0]);
    CheckExportToCsvIsInvoked(context_menu, app_, view_, BuildExpectedExportEventsToCsvString({1}),
                              orbit_data_views::kMenuActionExportEventsToCsv);
  }

  // Go to Disassembly
  {
    const int disassembly_index = GetActionIndexOnMenu(context_menu, kMenuActionDisassembly);
    EXPECT_TRUE(disassembly_index != kInvalidActionIndex);

    EXPECT_CALL(app_, Disassemble)
        .Times(1)
        .WillOnce([&](int32_t /*pid*/, const FunctionInfo& function) {
          EXPECT_EQ(function.pretty_name(), kFunctionPrettyNames[0]);
        });
    view_.OnContextMenu(std::string{kMenuActionDisassembly}, disassembly_index, {0});
  }

  // Go to Source code
  {
    const int source_code_index = GetActionIndexOnMenu(context_menu, kMenuActionSourceCode);
    EXPECT_TRUE(source_code_index != kInvalidActionIndex);

    EXPECT_CALL(app_, ShowSourceCode).Times(1).WillOnce([&](const FunctionInfo& function) {
      EXPECT_EQ(function.pretty_name(), kFunctionPrettyNames[0]);
    });
    view_.OnContextMenu(std::string{kMenuActionSourceCode}, source_code_index, {0});
  }

  // Hook
  {
    const int hook_index = GetActionIndexOnMenu(context_menu, kMenuActionSelect);
    EXPECT_TRUE(hook_index != kInvalidActionIndex);

    EXPECT_CALL(app_, SelectFunction).Times(1).WillOnce([&](const FunctionInfo& function) {
      EXPECT_EQ(function.pretty_name(), kFunctionPrettyNames[0]);
    });
    view_.OnContextMenu(std::string{kMenuActionSelect}, hook_index, {0});
  }

  function_selected = true;
  context_menu =
      FlattenContextMenuWithGroupingAndCheckOrder(view_.GetContextMenuWithGrouping(0, {0}));
  ASSERT_FALSE(context_menu.empty());

  // Unhook
  {
    const int unhook_index = GetActionIndexOnMenu(context_menu, kMenuActionUnselect);
    EXPECT_TRUE(unhook_index != kInvalidActionIndex);

    EXPECT_CALL(app_, DeselectFunction).Times(1).WillOnce([&](const FunctionInfo& function) {
      EXPECT_EQ(function.pretty_name(), kFunctionPrettyNames[0]);
    });
    view_.OnContextMenu(std::string{kMenuActionUnselect}, unhook_index, {0});
  }

  AddFunctionsByIndices({2});
  context_menu =
      FlattenContextMenuWithGroupingAndCheckOrder(view_.GetContextMenuWithGrouping(0, {0}));
  ASSERT_FALSE(context_menu.empty());

  // Load Symbols
  {
    const int load_symbols_index = GetActionIndexOnMenu(context_menu, kMenuActionLoadSymbols);
    EXPECT_TRUE(load_symbols_index != kInvalidActionIndex);

    EXPECT_CALL(app_, GetMutableModuleByModuleIdentifier)
        .Times(1)
        .WillOnce([&](const ModuleIdentifier& module_id) -> ModuleData* {
          EXPECT_EQ(module_id.build_id, kModuleBuildIds[2]);
          return module_manager_.GetMutableModuleByModuleIdentifier(module_id);
        });
    EXPECT_CALL(app_, LoadSymbolsManually)
        .Times(1)
        .WillOnce(testing::Return(orbit_base::Future<void>{}));
    view_.OnContextMenu(std::string{kMenuActionLoadSymbols}, load_symbols_index, {0});
  }
}

TEST_F(SamplingReportDataViewTest, OnSelectWillUpdateSamplingReport) {
  EXPECT_CALL(sampling_report_, OnSelectAddresses)
      .Times(1)
      .WillOnce([&](const absl::flat_hash_set<uint64_t>& addresses,
                    orbit_client_data::ThreadID /*thread_id*/) {
        EXPECT_THAT(addresses, testing::UnorderedElementsAre(kSampledAbsoluteAddresses[0]));
      });

  AddFunctionsByIndices({0});
  view_.OnSelect({0});
}

TEST_F(SamplingReportDataViewTest, OnRefreshMightUpdateSamplingReport) {
  AddFunctionsByIndices({0});

  // Test refresh triggered by sorting.
  {
    EXPECT_CALL(sampling_report_, OnSelectAddresses)
        .Times(1)
        .WillOnce([&](const absl::flat_hash_set<uint64_t>& addresses,
                      orbit_client_data::ThreadID /*thread_id*/) {
          EXPECT_THAT(addresses, testing::UnorderedElementsAre(kSampledAbsoluteAddresses[0]));
        });
    view_.OnRefresh({0}, RefreshMode::kOnSort);
  }

  // Test refresh triggered by filtering.
  {
    EXPECT_CALL(sampling_report_, OnSelectAddresses)
        .Times(1)
        .WillOnce([&](const absl::flat_hash_set<uint64_t>& addresses,
                      orbit_client_data::ThreadID /*thread_id*/) {
          EXPECT_THAT(addresses, testing::UnorderedElementsAre(kSampledAbsoluteAddresses[0]));
        });
    view_.OnRefresh({0}, RefreshMode::kOnFilter);
  }

  // Test refresh triggered by other cases.
  {
    EXPECT_CALL(sampling_report_, OnSelectAddresses).Times(0);
    view_.OnRefresh({0}, RefreshMode::kOther);
  }
}

TEST_F(SamplingReportDataViewTest, FilteringShowsRightResults) {
  AddFunctionsByIndices({0, 1, 2, 3});

  // Filtering by module name with single token
  {
    view_.OnFilter("f");
    EXPECT_EQ(view_.GetNumElements(), 2);
    EXPECT_THAT(
        (std::array{view_.GetValue(0, kColumnModuleName), view_.GetValue(1, kColumnModuleName)}),
        testing::UnorderedElementsAre(
            GetExpectedDisplayModuleNameByIndex(0, module_manager_, *capture_data_),
            GetExpectedDisplayModuleNameByIndex(2, module_manager_, *capture_data_)));
  }

  // Filtering by module name with multiple tokens separated by " ".
  {
    view_.OnFilter("foo module");
    EXPECT_EQ(view_.GetNumElements(), 1);
    EXPECT_EQ(view_.GetValue(0, kColumnModuleName),
              GetExpectedDisplayModuleNameByIndex(0, module_manager_, *capture_data_));
  }

  // No matching result
  {
    view_.OnFilter("abcdefg");
    EXPECT_EQ(view_.GetNumElements(), 0);
  }
}

TEST_F(SamplingReportDataViewTest, ColumnSortingShowsRightResults) {
  AddFunctionsByIndices({0, 1, 2});
  EXPECT_CALL(app_, HasCaptureData).WillRepeatedly(testing::Return(true));
  EXPECT_CALL(app_, GetCaptureData).WillRepeatedly(testing::ReturnRef(*capture_data_));

  using ViewRowEntry = std::array<std::string, kNumColumns>;
  std::vector<ViewRowEntry> view_entries;
  absl::flat_hash_map<std::string, uint64_t> string_to_raw_value;
  for (size_t i = 0; i < view_.GetNumElements(); i++) {
    ViewRowEntry entry;
    entry[kColumnFunctionName] =
        GetExpectedDisplayFunctionNameByIndex(i, module_manager_, *capture_data_);
    entry[kColumnModuleName] =
        GetExpectedDisplayModuleNameByIndex(i, module_manager_, *capture_data_);
    entry[kColumnExclusive] = GetExpectedDisplayExclusiveByIndex(i);
    string_to_raw_value.insert_or_assign(entry[kColumnExclusive], kSampledExclusives[i]);
    entry[kColumnInclusive] = GetExpectedDisplayInclusiveByIndex(i);
    string_to_raw_value.insert_or_assign(entry[kColumnInclusive], kSampledInclusives[i]);
    entry[kColumnUnwindErrors] = GetExpectedDisplayUnwindErrorsByIndex(i);
    string_to_raw_value.insert_or_assign(entry[kColumnUnwindErrors], kSampledUnwindErrors[i]);
    entry[kColumnAddress] = GetExpectedDisplayAddressByIndex(i);
    string_to_raw_value.insert_or_assign(entry[kColumnAddress], kSampledAbsoluteAddresses[i]);

    view_entries.push_back(entry);
  }

  auto sort_and_verify = [&](int column, orbit_data_views::DataView::SortingOrder order) {
    view_.OnSort(column, order);

    switch (column) {
      case kColumnFunctionName:
      case kColumnModuleName:
        // Columns sorted by display values (i.e., string).
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
      case kColumnExclusive:
      case kColumnInclusive:
      case kColumnUnwindErrors:
      case kColumnAddress:
        // Columns sorted by raw values (i.e., uint32_t / uint64_t).
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
      for (int column = kColumnFunctionName; column < kNumColumns; ++column) {
        EXPECT_EQ(view_.GetValue(index, column), view_entries[index][column]);
      }
    }
  };

  for (int column = kColumnFunctionName; column < kNumColumns; ++column) {
    // Sort by ascending
    { sort_and_verify(column, orbit_data_views::DataView::SortingOrder::kAscending); }

    // Sort by descending
    { sort_and_verify(column, orbit_data_views::DataView::SortingOrder::kDescending); }
  }
}