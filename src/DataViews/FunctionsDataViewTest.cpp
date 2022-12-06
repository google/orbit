// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/hash/hash.h>
#include <absl/strings/ascii.h>
#include <absl/strings/str_format.h>
#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ClientData/CaptureData.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ProcessData.h"
#include "DataViewTestUtils.h"
#include "DataViews/DataView.h"
#include "DataViews/FunctionsDataView.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/process.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "MockAppInterface.h"

using orbit_client_data::CaptureData;
using orbit_client_data::FunctionInfo;
using orbit_data_views::CheckCopySelectionIsInvoked;
using orbit_data_views::CheckExportToCsvIsInvoked;
using orbit_data_views::CheckSingleAction;
using orbit_data_views::ContextMenuEntry;
using orbit_data_views::FlattenContextMenu;
using orbit_data_views::FlattenContextMenuWithGroupingAndCheckOrder;
using orbit_data_views::kMenuActionCopySelection;
using orbit_data_views::kMenuActionDisableFrameTrack;
using orbit_data_views::kMenuActionDisassembly;
using orbit_data_views::kMenuActionEnableFrameTrack;
using orbit_data_views::kMenuActionExportToCsv;
using orbit_data_views::kMenuActionSelect;
using orbit_data_views::kMenuActionSourceCode;
using orbit_data_views::kMenuActionUnselect;
using orbit_grpc_protos::ModuleInfo;

using ::testing::Return;

namespace {
struct FunctionsDataViewTest : public testing::Test {
 public:
  explicit FunctionsDataViewTest() : view_{&app_} {
    view_.Init();
    FunctionInfo function0{"/path/to/module", "buildid", /*address=*/12,
                           /*size=*/16,       "foo()",   /*is_hotpatchable=*/false};
    functions_.emplace_back(std::move(function0));

    FunctionInfo function1{"path/to/other", "buildid1",          /*address=*/0x100,
                           /*size=*/42,     "main(int, char**)", /*is_hotpatchable=*/false};
    functions_.emplace_back(std::move(function1));

    FunctionInfo function2{"/somewhere/else/module",
                           "buildid2",
                           /*address=*/0x330,
                           /*size=*/66,
                           "operator==(A const&, A const&)",
                           /*is_hotpatchable=*/false};
    functions_.emplace_back(std::move(function2));

    FunctionInfo function3{"/somewhere/else/CapitalizedModule",
                           "buildid3",
                           /*address=*/0x33,
                           /*size=*/66,
                           "ffind(int)",
                           /*is_hotpatchable=*/false};
    functions_.emplace_back(std::move(function3));

    FunctionInfo function4{"/somewhere/else/UPPERCASEMODULE",
                           "buildid4",
                           /*address=*/0x33,
                           /*size=*/66,
                           "bar(const char*)",
                           /*is_hotpatchable=*/false};
    functions_.emplace_back(std::move(function4));

    ModuleInfo module_info0{};
    module_info0.set_file_path(functions_[0].module_path());
    module_info0.set_file_size(0x42);
    module_info0.set_build_id(functions_[0].module_build_id());
    module_info0.set_load_bias(0x4000);
    module_info0.set_address_start(0x1234);
    module_infos_.emplace_back(std::move(module_info0));

    ModuleInfo module_info1{};
    module_info1.set_file_path(functions_[1].module_path());
    module_info1.set_file_size(0x24);
    module_info1.set_build_id(functions_[1].module_build_id());
    module_info1.set_load_bias(0x5000);
    module_info1.set_address_start(0x2345);
    module_infos_.emplace_back(std::move(module_info1));

    ModuleInfo module_info2{};
    module_info2.set_file_path(functions_[2].module_path());
    module_info2.set_file_size(0x55);
    module_info2.set_build_id(functions_[2].module_build_id());
    module_info2.set_load_bias(0x6000);
    module_info2.set_address_start(0x3456);
    module_infos_.emplace_back(std::move(module_info2));
  }

 protected:
  orbit_data_views::MockAppInterface app_;
  orbit_data_views::FunctionsDataView view_;
  std::vector<FunctionInfo> functions_;
  std::vector<ModuleInfo> module_infos_;

  [[nodiscard]] std::optional<size_t> IndexOfFunction(const FunctionInfo& function) const {
    const auto it =
        std::find_if(functions_.begin(), functions_.end(), [&](const FunctionInfo& candidate) {
          // This is not a canonical comparison, but since we control
          // our testing data, we can assure that all our functions have
          // distinctive names.
          return function.pretty_name() == candidate.pretty_name();
        });
    if (it == functions_.end()) return std::nullopt;
    return std::distance(functions_.begin(), it);
  }
};
}  // namespace

TEST_F(FunctionsDataViewTest, ColumnHeadersNotEmpty) {
  EXPECT_GE(view_.GetColumns().size(), 1);
  for (const auto& column : view_.GetColumns()) {
    EXPECT_FALSE(column.header.empty());
  }
}

TEST_F(FunctionsDataViewTest, HasValidDefaultSortingColumn) {
  EXPECT_GE(view_.GetDefaultSortingColumn(), 0);
  EXPECT_LT(view_.GetDefaultSortingColumn(), view_.GetColumns().size());
}

TEST_F(FunctionsDataViewTest, IsEmptyOnConstruction) {
  EXPECT_EQ(view_.GetNumElements(), 0);
  EXPECT_EQ(view_.GetLabel(), "Functions");
}

TEST_F(FunctionsDataViewTest, FunctionNameIsDisplayName) {
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const FunctionInfo&>()))
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  view_.AddFunctions({&functions_[0]});
  ASSERT_EQ(view_.GetNumElements(), 1);
  EXPECT_EQ(view_.GetValue(0, 1), functions_[0].pretty_name());
}

TEST_F(FunctionsDataViewTest, InvalidColumnAndRowNumbersReturnEmptyString) {
  view_.AddFunctions({&functions_[0]});
  ASSERT_EQ(view_.GetNumElements(), 1);
  EXPECT_EQ(view_.GetValue(1, 0), "");    // Invalid row index
  EXPECT_EQ(view_.GetValue(0, 25), "");   // Invalid column index
  EXPECT_EQ(view_.GetValue(42, 25), "");  // Invalid column and row index
}

TEST_F(FunctionsDataViewTest, ViewHandlesMultipleElements) {
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const FunctionInfo&>()))
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  view_.AddFunctions({&functions_[0], &functions_[1], &functions_[2]});
  ASSERT_EQ(view_.GetNumElements(), 3);

  // We don't expect the view to be in any particular order at this point.
  EXPECT_THAT(
      (std::array{view_.GetValue(0, 1), view_.GetValue(1, 1), view_.GetValue(2, 1)}),
      testing::UnorderedElementsAre(functions_[0].pretty_name(), functions_[1].pretty_name(),
                                    functions_[2].pretty_name()));
}

TEST_F(FunctionsDataViewTest, ClearFunctionsRemovesAllElements) {
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const FunctionInfo&>()))
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  view_.AddFunctions({&functions_[0], &functions_[1], &functions_[2]});
  ASSERT_EQ(view_.GetNumElements(), 3);

  view_.ClearFunctions();
  ASSERT_EQ(view_.GetNumElements(), 0);
}

TEST_F(FunctionsDataViewTest, RemoveFunctionsOfModule) {
  view_.AddFunctions(
      {&functions_[0], &functions_[1], &functions_[2], &functions_[3], &functions_[4]});
  ASSERT_EQ(view_.GetNumElements(), 5);

  view_.RemoveFunctionsOfModule(functions_[2].module_path());
  ASSERT_EQ(view_.GetNumElements(), 4);
  EXPECT_THAT(
      (std::array{view_.GetValue(0, 1), view_.GetValue(1, 1), view_.GetValue(2, 1),
                  view_.GetValue(3, 1)}),
      testing::UnorderedElementsAre(functions_[0].pretty_name(), functions_[1].pretty_name(),
                                    functions_[3].pretty_name(), functions_[4].pretty_name()));

  view_.RemoveFunctionsOfModule(functions_[3].module_path());
  ASSERT_EQ(view_.GetNumElements(), 3);
  EXPECT_THAT(
      (std::array{view_.GetValue(0, 1), view_.GetValue(1, 1), view_.GetValue(2, 1)}),
      testing::UnorderedElementsAre(functions_[0].pretty_name(), functions_[1].pretty_name(),
                                    functions_[4].pretty_name()));

  view_.RemoveFunctionsOfModule(functions_[3].module_path());  // Should do nothing.
  ASSERT_EQ(view_.GetNumElements(), 3);
}

const std::string kSelectedFunctionString = "H";

TEST_F(FunctionsDataViewTest, FunctionSelectionAppearsInFirstColumn) {
  bool function_selected = false;
  bool frame_track_enabled = false;

  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const FunctionInfo&>()))
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::ReturnPointee(&function_selected));

  // We have the frame track handling in here, but we won't test if it works correctly. There is a
  // separate test for this.
  EXPECT_CALL(app_, IsFrameTrackEnabled)
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::ReturnPointee(&frame_track_enabled));

  EXPECT_CALL(app_, HasCaptureData).WillRepeatedly(testing::Return(false));

  view_.AddFunctions({&functions_[0]});
  ASSERT_EQ(view_.GetNumElements(), 1);

  function_selected = false;
  frame_track_enabled = false;
  EXPECT_THAT(view_.GetValue(0, 0), testing::Not(testing::StartsWith(kSelectedFunctionString)));

  function_selected = true;
  frame_track_enabled = false;
  EXPECT_THAT(view_.GetValue(0, 0), testing::StartsWith(kSelectedFunctionString));

  function_selected = false;
  frame_track_enabled = false;
  EXPECT_THAT(view_.GetValue(0, 0), testing::Not(testing::StartsWith(kSelectedFunctionString)));

  function_selected = true;
  frame_track_enabled = true;
  EXPECT_THAT(view_.GetValue(0, 0), testing::StartsWith(kSelectedFunctionString));
}

TEST_F(FunctionsDataViewTest, FrameTrackSelectionAppearsInFirstColumn) {
  bool function_selected = false;
  bool frame_track_enabled = false;

  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const FunctionInfo&>()))
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::ReturnPointee(&function_selected));

  EXPECT_CALL(app_, IsFrameTrackEnabled)
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::ReturnPointee(&frame_track_enabled));

  EXPECT_CALL(app_, HasCaptureData).WillRepeatedly(testing::Return(false));

  view_.AddFunctions({&functions_[0]});
  ASSERT_EQ(view_.GetNumElements(), 1);

  function_selected = false;
  frame_track_enabled = false;
  EXPECT_THAT(view_.GetValue(0, 0), testing::Not(testing::EndsWith("F")));

  function_selected = true;
  frame_track_enabled = true;
  EXPECT_THAT(view_.GetValue(0, 0), testing::EndsWith("F"));

  function_selected = true;
  frame_track_enabled = false;
  EXPECT_THAT(view_.GetValue(0, 0), testing::Not(testing::EndsWith("F")));
}

TEST_F(FunctionsDataViewTest, FrameTrackSelectionAppearsInFirstColumnWhenACaptureWasLoaded) {
  // There is a second way a frame track can be considered enabled: When we have a capture loaded
  // and the frame track is part of the capture.
  // Since CaptureData is entangled with ModuleManager the test needs to create a lot of empty
  // data structures and manager objects.

  orbit_client_data::ModuleManager module_manager{};
  EXPECT_CALL(app_, GetModuleManager()).WillRepeatedly(Return(&module_manager));
  (void)module_manager.AddOrUpdateModules({module_infos_[0]});
  ASSERT_EQ(module_manager.GetAllModuleData().size(), 1);

  orbit_grpc_protos::SymbolInfo symbol_info;
  symbol_info.set_demangled_name(functions_[0].pretty_name());
  symbol_info.set_address(functions_[0].address());
  symbol_info.set_size(functions_[0].size());
  orbit_grpc_protos::ModuleSymbols module_symbols;
  module_symbols.mutable_symbol_infos()->Add(std::move(symbol_info));
  orbit_client_data::ModuleData* module_data =
      module_manager.GetMutableModuleByModuleIdentifier(functions_[0].module_id());
  module_data->AddSymbols(module_symbols);

  orbit_grpc_protos::CaptureStarted capture_started{};

  orbit_grpc_protos::InstrumentedFunction* instrumented_function =
      capture_started.mutable_capture_options()->add_instrumented_functions();
  instrumented_function->set_file_path(functions_[0].module_path());
  instrumented_function->set_file_build_id(functions_[0].module_build_id());
  instrumented_function->set_file_offset(functions_[0].ComputeFileOffset(*module_data));
  instrumented_function->set_function_virtual_address(functions_[0].address());

  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const FunctionInfo&>()))
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(true));

  // We return false here, since we test the second way frame tracks are considered enabled.
  EXPECT_CALL(app_, IsFrameTrackEnabled)
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  EXPECT_CALL(app_, HasCaptureData).Times(2).WillRepeatedly(testing::Return(true));

  CaptureData capture_data{
      capture_started, std::nullopt, {}, CaptureData::DataSource::kLiveCapture};
  EXPECT_CALL(app_, GetCaptureData).Times(2).WillRepeatedly(testing::ReturnPointee(&capture_data));

  // Note that `CaptureData` also keeps a list of enabled frame track function ids, but this list is
  // not used to determine whether a frame track was enabled for a capture. `FunctionsDataView`
  // calls `OrbitApp::HasFrameTrackInCaptureData` (which calls some function in `TimeGraph` instead
  // in the non-mock implementation.)
  bool frame_track_enabled = false;
  EXPECT_CALL(app_, HasFrameTrackInCaptureData)
      .Times(2)
      .WillRepeatedly(testing::ReturnPointee(&frame_track_enabled));

  view_.AddFunctions({&functions_[0]});
  ASSERT_EQ(view_.GetNumElements(), 1);

  frame_track_enabled = true;
  EXPECT_THAT(view_.GetValue(0, 0), testing::EndsWith("F"));

  frame_track_enabled = false;
  EXPECT_THAT(view_.GetValue(0, 0), testing::Not(testing::EndsWith("F")));
}

TEST_F(FunctionsDataViewTest, FunctionSizeAppearsInThirdColumn) {
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const FunctionInfo&>()))
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  view_.AddFunctions({&functions_[0]});
  ASSERT_EQ(view_.GetNumElements(), 1);
  EXPECT_EQ(view_.GetValue(0, 2), std::to_string(functions_[0].size()));
}

TEST_F(FunctionsDataViewTest, ModuleColumnShowsFilenameOfModule) {
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const FunctionInfo&>()))
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  view_.AddFunctions({&functions_[0]});
  ASSERT_EQ(view_.GetNumElements(), 1);
  EXPECT_EQ(view_.GetValue(0, 3),
            std::filesystem::path{functions_[0].module_path()}.filename().string());
}

TEST_F(FunctionsDataViewTest, AddressColumnShowsAddress) {
  view_.AddFunctions({&functions_[0]});
  ASSERT_EQ(view_.GetNumElements(), 1);

  // We expect the address to be in hex - indicated by "0x"
  EXPECT_THAT(view_.GetValue(0, 4), testing::StartsWith("0x"));

  EXPECT_EQ(view_.GetValue(0, 4).substr(2), absl::StrFormat("%x", functions_[0].address()));
}

TEST_F(FunctionsDataViewTest, ContextMenuEntriesChangeOnFunctionState) {
  std::array<bool, 3> is_function_selected = {true, true, false};
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const FunctionInfo&>()))
      .Times(testing::AnyNumber())
      .WillRepeatedly([&](const FunctionInfo& function) -> bool {
        std::optional<size_t> index = IndexOfFunction(function);
        EXPECT_TRUE(index.has_value());
        return is_function_selected.at(index.value());
      });

  std::array<bool, 3> is_frame_track_enabled = {true, false, false};
  EXPECT_CALL(app_, IsFrameTrackEnabled)
      .Times(testing::AnyNumber())
      .WillRepeatedly([&](const FunctionInfo& function) -> bool {
        std::optional<size_t> index = IndexOfFunction(function);
        EXPECT_TRUE(index.has_value());
        return is_frame_track_enabled.at(index.value());
      });

  view_.AddFunctions({&functions_[0], &functions_[1], &functions_[2]});

  auto verify_context_menu_action_availability = [&](absl::Span<const int> selected_indices) {
    FlattenContextMenu context_menu = FlattenContextMenuWithGroupingAndCheckOrder(
        view_.GetContextMenuWithGrouping(0, selected_indices));

    // Common actions should always be available.
    CheckSingleAction(context_menu, kMenuActionCopySelection, ContextMenuEntry::kEnabled);
    CheckSingleAction(context_menu, kMenuActionExportToCsv, ContextMenuEntry::kEnabled);

    // Source code and disassembly actions are also always available.
    CheckSingleAction(context_menu, kMenuActionSourceCode, ContextMenuEntry::kEnabled);
    CheckSingleAction(context_menu, kMenuActionDisassembly, ContextMenuEntry::kEnabled);

    // Hook action is available if and only if there is an unselected function. Unhook action is
    // available if and only if there is a selected instrumented function.
    // Enable frametrack action is available if and only if there is a function with frametrack not
    // yet enabled, disable frametrack action is available if and only if there is a function with
    // frametrack enabled.
    ContextMenuEntry select = ContextMenuEntry::kDisabled;
    ContextMenuEntry unselect = ContextMenuEntry::kDisabled;
    ContextMenuEntry enable_frametrack = ContextMenuEntry::kDisabled;
    ContextMenuEntry disable_frametrack = ContextMenuEntry::kDisabled;
    for (size_t index : selected_indices) {
      if (is_function_selected.at(index)) {
        unselect = ContextMenuEntry::kEnabled;
      } else {
        select = ContextMenuEntry::kEnabled;
      }

      if (is_frame_track_enabled.at(index)) {
        disable_frametrack = ContextMenuEntry::kEnabled;
      } else {
        enable_frametrack = ContextMenuEntry::kEnabled;
      }
    }
    CheckSingleAction(context_menu, kMenuActionSelect, select);
    CheckSingleAction(context_menu, kMenuActionUnselect, unselect);
    CheckSingleAction(context_menu, kMenuActionEnableFrameTrack, enable_frametrack);
    CheckSingleAction(context_menu, kMenuActionDisableFrameTrack, disable_frametrack);
  };

  verify_context_menu_action_availability({0});
  verify_context_menu_action_availability({1});
  verify_context_menu_action_availability({2});
  verify_context_menu_action_availability({0, 1, 2});
}

TEST_F(FunctionsDataViewTest, GenericDataExportFunctionShowCorrectData) {
  // This functionality is not tested in this test case.
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const FunctionInfo&>()))
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  // This functionality is not tested in this test case.
  EXPECT_CALL(app_, IsFrameTrackEnabled)
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  // This functionality is not tested in this test case.
  EXPECT_CALL(app_, HasCaptureData)
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  view_.AddFunctions({&functions_[0]});

  FlattenContextMenu context_menu =
      FlattenContextMenuWithGroupingAndCheckOrder(view_.GetContextMenuWithGrouping(0, {0}));

  // Copy Selection
  {
    std::string expected_clipboard = absl::StrFormat(
        "Hooked\tFunction\tSize\tModule\tAddress in module\n"
        "\t%s\t%d\t%s\t%#x\n",
        functions_[0].pretty_name(), functions_[0].size(),
        std::filesystem::path{functions_[0].module_path()}.filename().string(),
        functions_[0].address());
    CheckCopySelectionIsInvoked(context_menu, app_, view_, expected_clipboard);
  }

  // Export to CSV
  {
    std::string expected_contents =
        absl::StrFormat(R"("Hooked","Function","Size","Module","Address in module")"
                        "\r\n"
                        R"("","%s","%d","%s","%#x")"
                        "\r\n",
                        functions_[0].pretty_name(), functions_[0].size(),
                        std::filesystem::path{functions_[0].module_path()}.filename().string(),
                        functions_[0].address());
    CheckExportToCsvIsInvoked(context_menu, app_, view_, expected_contents);
  }
}

TEST_F(FunctionsDataViewTest, ColumnSorting) {
  // This functionality is not tested in this test case.
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const FunctionInfo&>()))
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  // This functionality is not tested in this test case.
  EXPECT_CALL(app_, IsFrameTrackEnabled)
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  // This functionality is not tested in this test case.
  EXPECT_CALL(app_, HasCaptureData)
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  // Note that FunctionsDataView also has constants defined for its columns, but these are declared
  // protected.
  constexpr int kNameColumn = 1;
  constexpr int kSizeColumn = 2;
  constexpr int kModuleColumn = 3;
  constexpr int kAddressColumn = 4;
  EXPECT_EQ(view_.GetDefaultSortingColumn(), kAddressColumn);

  std::vector<FunctionInfo> functions = functions_;
  view_.AddFunctions(
      {&functions_[0], &functions_[1], &functions_[2], &functions_[3], &functions_[4]});

  const auto verify_correct_sorting = [&]() {
    // We won't check all columns because we control the test data and know that checking address
    // and name is enough to ensure that it's sorted properly.
    for (size_t idx = 0; idx < functions.size(); ++idx) {
      EXPECT_EQ(view_.GetValue(idx, kAddressColumn),
                absl::StrFormat("%#x", functions[idx].address()));
      EXPECT_EQ(view_.GetValue(idx, 1), functions[idx].pretty_name());
    }
  };

  // Sort by name ascending
  view_.OnSort(kNameColumn, orbit_data_views::DataView::SortingOrder::kAscending);
  std::sort(functions.begin(), functions.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.pretty_name() < rhs.pretty_name(); });
  verify_correct_sorting();

  // Sort by name descending
  view_.OnSort(kNameColumn, orbit_data_views::DataView::SortingOrder::kDescending);
  std::sort(functions.begin(), functions.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.pretty_name() > rhs.pretty_name(); });
  verify_correct_sorting();

  // Sort by size ascending
  view_.OnSort(kSizeColumn, orbit_data_views::DataView::SortingOrder::kAscending);
  std::sort(functions.begin(), functions.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.size() < rhs.size(); });
  verify_correct_sorting();

  // Sort by size descending
  view_.OnSort(kSizeColumn, orbit_data_views::DataView::SortingOrder::kDescending);
  std::sort(functions.begin(), functions.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.size() > rhs.size(); });
  verify_correct_sorting();

  // Sort by module ascending
  view_.OnSort(kModuleColumn, orbit_data_views::DataView::SortingOrder::kAscending);
  std::sort(functions.begin(), functions.end(), [](const auto& lhs, const auto& rhs) {
    return std::filesystem::path{lhs.module_path()}.filename().string() <
           std::filesystem::path{rhs.module_path()}.filename().string();
  });
  verify_correct_sorting();

  // Sort by module descending
  view_.OnSort(kModuleColumn, orbit_data_views::DataView::SortingOrder::kDescending);
  std::sort(functions.begin(), functions.end(), [](const auto& lhs, const auto& rhs) {
    return std::filesystem::path{lhs.module_path()}.filename().string() >
           std::filesystem::path{rhs.module_path()}.filename().string();
  });
  verify_correct_sorting();

  // Default sorting is broken in DataView, so let's explicitly sort here. This will be fixed later.
  view_.OnSort(kAddressColumn, orbit_data_views::DataView::SortingOrder::kAscending);
  std::sort(functions.begin(), functions.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.address() < rhs.address(); });
  verify_correct_sorting();

  // Sort by address descending
  view_.OnSort(kAddressColumn, orbit_data_views::DataView::SortingOrder::kDescending);
  std::sort(functions.begin(), functions.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.address() > rhs.address(); });
  verify_correct_sorting();
}

TEST_F(FunctionsDataViewTest, ContextMenuActionsCallCorrespondingFunctionsInAppInterface) {
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const FunctionInfo&>()))
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));
  EXPECT_CALL(app_, IsFrameTrackEnabled)
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  CaptureData capture_data{{}, std::nullopt, {}, CaptureData::DataSource::kLiveCapture};
  EXPECT_CALL(app_, GetCaptureData).WillRepeatedly(testing::ReturnPointee(&capture_data));
  EXPECT_CALL(app_, IsCaptureConnected).WillRepeatedly(testing::Return(true));
  EXPECT_CALL(app_, HasCaptureData).WillRepeatedly(testing::Return(true));

  view_.AddFunctions({&functions_[0]});

  const auto match_function = [&](const FunctionInfo& function) {
    EXPECT_EQ(function.address(), functions_[0].address());
    EXPECT_EQ(function.pretty_name(), functions_[0].pretty_name());
  };

  EXPECT_CALL(app_, SelectFunction).Times(1).WillRepeatedly(match_function);
  view_.OnContextMenu(std::string{kMenuActionSelect}, 0, {0});

  EXPECT_CALL(app_, DeselectFunction).Times(1).WillRepeatedly(match_function);
  EXPECT_CALL(app_, DisableFrameTrack).Times(1).WillRepeatedly(match_function);
  EXPECT_CALL(app_, RemoveFrameTrack).Times(1).WillRepeatedly([&](const FunctionInfo& function) {
    match_function(function);
  });
  view_.OnContextMenu(std::string{kMenuActionUnselect}, 0, {0});

  EXPECT_CALL(app_, SelectFunction).Times(1).WillRepeatedly(match_function);
  EXPECT_CALL(app_, EnableFrameTrack).Times(1).WillRepeatedly(match_function);
  EXPECT_CALL(app_, AddFrameTrack).Times(1).WillRepeatedly([&](const FunctionInfo& function) {
    match_function(function);
  });
  view_.OnContextMenu(std::string{kMenuActionEnableFrameTrack}, 0, {0});

  EXPECT_CALL(app_, DisableFrameTrack).Times(1).WillRepeatedly(match_function);
  EXPECT_CALL(app_, RemoveFrameTrack).Times(1).WillRepeatedly([&](const FunctionInfo& function) {
    match_function(function);
  });
  view_.OnContextMenu(std::string{kMenuActionDisableFrameTrack}, 0, {0});

  constexpr int kRandomPid = 4242;
  orbit_grpc_protos::ProcessInfo process_info{};
  process_info.set_pid(kRandomPid);
  orbit_client_data::ProcessData process_data{process_info};

  EXPECT_CALL(app_, GetTargetProcess).Times(1).WillRepeatedly(testing::Return(&process_data));
  EXPECT_CALL(app_, Disassemble)
      .Times(1)
      .WillRepeatedly([&](int pid, const FunctionInfo& function) {
        EXPECT_EQ(pid, kRandomPid);
        match_function(function);
      });
  view_.OnContextMenu(std::string{kMenuActionDisassembly}, 0, {0});

  EXPECT_CALL(app_, ShowSourceCode).Times(1).WillRepeatedly(match_function);
  view_.OnContextMenu(std::string{kMenuActionSourceCode}, 0, {0});
}

TEST_F(FunctionsDataViewTest, FilteringByFunctionName) {
  // This functionality is not tested in this test case.
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const FunctionInfo&>()))
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  // This functionality is not tested in this test case.
  EXPECT_CALL(app_, IsFrameTrackEnabled)
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  // This functionality is not tested in this test case.
  EXPECT_CALL(app_, HasCaptureData)
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  view_.AddFunctions(
      {&functions_[0], &functions_[1], &functions_[2], &functions_[3], &functions_[4]});

  // Filtering by an empty string should result in all functions listed -> No filtering.
  view_.OnFilter("");
  EXPECT_EQ(view_.GetNumElements(), functions_.size());

  // We know that the function name of function 3 is unique, so we expect only the very same
  // function as the filter result.
  view_.OnFilter(functions_[3].pretty_name());
  EXPECT_EQ(view_.GetNumElements(), 1);
  EXPECT_EQ(view_.GetValue(0, 1), functions_[3].pretty_name());

  // We know that the function name of function 4 is unique, so we expect only the very same
  // function as the filter result.
  view_.OnFilter(functions_[4].pretty_name());
  EXPECT_EQ(view_.GetNumElements(), 1);
  EXPECT_EQ(view_.GetValue(0, 1), functions_[4].pretty_name());

  // The token `f` only appears in function 0 (foo) and 3 (ffind).
  view_.OnFilter("f");
  EXPECT_EQ(view_.GetNumElements(), 2);
  EXPECT_THAT(
      (std::array{view_.GetValue(0, 1), view_.GetValue(1, 1)}),
      testing::UnorderedElementsAre(functions_[0].pretty_name(), functions_[3].pretty_name()));

  // The token `ff` only appears in function 3 (ffind) while `in` appears both in function 1 (main)
  // and 3 (ffind). Nevertheless the result should only list function 3 since all tokens are
  // required to appear.
  view_.OnFilter("ff in");
  EXPECT_EQ(view_.GetNumElements(), 1);
  EXPECT_EQ(view_.GetValue(0, 1), functions_[3].pretty_name());

  // The same as the previous check, but with the tokens swapped.
  view_.OnFilter("in ff");
  EXPECT_EQ(view_.GetNumElements(), 1);
  EXPECT_EQ(view_.GetValue(0, 1), functions_[3].pretty_name());
}

TEST_F(FunctionsDataViewTest, FilteringByModuleName) {
  // This functionality is not tested in this test case.
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const FunctionInfo&>()))
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  // This functionality is not tested in this test case.
  EXPECT_CALL(app_, IsFrameTrackEnabled)
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  // This functionality is not tested in this test case.
  EXPECT_CALL(app_, HasCaptureData)
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  view_.AddFunctions(
      {&functions_[0], &functions_[1], &functions_[2], &functions_[3], &functions_[4]});

  // Only the filename is considered when filtering, so searching for the full file path results in
  // an empty search result.
  view_.OnFilter(functions_[4].module_path());
  EXPECT_EQ(view_.GetNumElements(), 0);

  view_.OnFilter(std::filesystem::path{functions_[4].module_path()}.filename().string());
  EXPECT_EQ(view_.GetNumElements(), 1);
  EXPECT_EQ(view_.GetValue(0, 1), functions_[4].pretty_name());

  view_.OnFilter(absl::AsciiStrToLower(
      std::filesystem::path{functions_[4].module_path()}.filename().string()));
  EXPECT_EQ(view_.GetNumElements(), 1);
  EXPECT_EQ(view_.GetValue(0, 1), functions_[4].pretty_name());
}

TEST_F(FunctionsDataViewTest, FilteringByFunctionAndModuleName) {
  // This functionality is not tested in this test case.
  EXPECT_CALL(app_, IsFunctionSelected(testing::A<const FunctionInfo&>()))
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  // This functionality is not tested in this test case.
  EXPECT_CALL(app_, IsFrameTrackEnabled)
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  // This functionality is not tested in this test case.
  EXPECT_CALL(app_, HasCaptureData)
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(false));

  view_.AddFunctions(
      {&functions_[0], &functions_[1], &functions_[2], &functions_[3], &functions_[4]});

  // ffind is the name of the function while CapitalizedModule is the filename of its module.
  view_.OnFilter("ffind CapitalizedModule");
  EXPECT_EQ(view_.GetNumElements(), 1);
  EXPECT_EQ(view_.GetValue(0, 1), functions_[3].pretty_name());

  // No results when joining the tokens
  view_.OnFilter("ffindCapitalizedModule");
  EXPECT_EQ(view_.GetNumElements(), 0);
}
