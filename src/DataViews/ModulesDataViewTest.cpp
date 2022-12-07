// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "ClientData/ModuleData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ProcessData.h"
#include "DataViewTestUtils.h"
#include "DataViews/DataView.h"
#include "DataViews/ModulesDataView.h"
#include "DataViews/SymbolLoadingState.h"
#include "DisplayFormats/DisplayFormats.h"
#include "GrpcProtos/module.pb.h"
#include "MockAppInterface.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"

namespace orbit_data_views {

using orbit_client_data::ModuleData;
using orbit_client_data::ModuleInMemory;
using orbit_grpc_protos::ModuleInfo;

namespace {

constexpr size_t kNumModules = 3;
constexpr std::array<uint64_t, kNumModules> kStartAddresses{0x1000, 0x2000, 0x3000};
constexpr std::array<uint64_t, kNumModules> kEndAddresses{0x1100, 0x2100, 0x3100};
constexpr std::array<uint64_t, kNumModules> kSizes{300, 100, 200};
const std::array<std::string, kNumModules> kNames{"module_abc", "module_abc", "module_xyz"};
const std::array<std::string, kNumModules> kFilePaths{
    "/usr/subpath/to/module_abc", "/local/subpath/to/module_abc", "/usr/subpath/to/module_xyz"};
const std::array<std::string, kNumModules> kBuildIds{"build_id_0", "build_id_1", "build_id_2"};

// ModulesDataView also has column index constants defined, but they are declared as private.
constexpr int kColumnSymbols = 0;
constexpr int kColumnName = 1;
constexpr int kColumnPath = 2;
constexpr int kColumnAddressRange = 3;
constexpr int kColumnFileSize = 4;
constexpr int kNumColumns = 5;

std::string GetExpectedDisplayAddressRangeByIndex(size_t index) {
  return absl::StrFormat("[%016x - %016x]", kStartAddresses[index], kEndAddresses[index]);
}

std::string GetExpectedDisplayFileSizeByIndex(size_t index) {
  return orbit_display_formats::GetDisplaySize(kSizes[index]);
}

class ModulesDataViewTest : public testing::Test {
 public:
  explicit ModulesDataViewTest() : view_{&app_} {
    view_.Init();
    for (size_t i = 0; i < kNumModules; i++) {
      ModuleInMemory module_in_memory{kStartAddresses[i], kEndAddresses[i], kFilePaths[i],
                                      kBuildIds[i]};
      modules_in_memory_.push_back(module_in_memory);

      ModuleInfo module_info{};
      module_info.set_name(kNames[i]);
      module_info.set_file_path(kFilePaths[i]);
      module_info.set_build_id(kBuildIds[i]);
      module_info.set_file_size(kSizes[i]);
      EXPECT_TRUE(module_manager_.AddOrUpdateModules({module_info}).empty());
    }
  }

  void AddModulesByIndices(absl::Span<const size_t> indices) {
    std::set index_set(indices.begin(), indices.end());
    for (size_t index : index_set) {
      ORBIT_CHECK(index < kNumModules);
      view_.AddModule(
          modules_in_memory_[index].start(),
          module_manager_.GetMutableModuleByModuleIdentifier(modules_in_memory_[index].module_id()),
          modules_in_memory_[index]);
    }
  }

 protected:
  orbit_data_views::MockAppInterface app_;
  orbit_data_views::ModulesDataView view_;
  orbit_client_data::ModuleManager module_manager_;
  std::vector<ModuleInMemory> modules_in_memory_;
};

}  // namespace

TEST_F(ModulesDataViewTest, ColumnHeadersNotEmpty) {
  EXPECT_GE(view_.GetColumns().size(), 1);
  for (const auto& column : view_.GetColumns()) {
    EXPECT_FALSE(column.header.empty());
  }
}

TEST_F(ModulesDataViewTest, HasValidDefaultSortingColumn) {
  EXPECT_GE(view_.GetDefaultSortingColumn(), kColumnFileSize);
  EXPECT_LT(view_.GetDefaultSortingColumn(), view_.GetColumns().size());
}

TEST_F(ModulesDataViewTest, ColumnValuesAreCorrect) {
  AddModulesByIndices({0});

  EXPECT_CALL(app_, GetSymbolLoadingStateForModule)
      .WillOnce(testing::Return(SymbolLoadingState::kUnknown));

  EXPECT_EQ(view_.GetValue(0, kColumnName), kNames[0]);
  EXPECT_EQ(view_.GetValue(0, kColumnPath), kFilePaths[0]);
  EXPECT_EQ(view_.GetValue(0, kColumnAddressRange), GetExpectedDisplayAddressRangeByIndex(0));
  EXPECT_EQ(view_.GetValue(0, kColumnFileSize), GetExpectedDisplayFileSizeByIndex(0));
  EXPECT_EQ(view_.GetValue(0, kColumnSymbols), "");
}

TEST_F(ModulesDataViewTest, ContextMenuEntriesArePresent) {
  constexpr size_t kIndex = 0;

  AddModulesByIndices({kIndex});
  FlattenContextMenu context_menu =
      FlattenContextMenuWithGroupingAndCheckOrder(view_.GetContextMenuWithGrouping(0, {kIndex}));

  CheckSingleAction(context_menu, kMenuActionCopySelection, ContextMenuEntry::kEnabled);
  CheckSingleAction(context_menu, kMenuActionExportToCsv, ContextMenuEntry::kEnabled);
  CheckSingleAction(context_menu, kMenuActionLoadSymbols, ContextMenuEntry::kEnabled);

  // Load Symbols should be disabled when module is loaded successfully.

  const ModuleInMemory& module_in_memory = modules_in_memory_[kIndex];
  ModuleData* module =
      module_manager_.GetMutableModuleByModuleIdentifier(module_in_memory.module_id());

  module->AddFallbackSymbols({});
  EXPECT_EQ(module->GetLoadedSymbolsCompleteness(),
            ModuleData::SymbolCompleteness::kDynamicLinkingAndUnwindInfo);
  context_menu =
      FlattenContextMenuWithGroupingAndCheckOrder(view_.GetContextMenuWithGrouping(0, {kIndex}));
  CheckSingleAction(context_menu, kMenuActionLoadSymbols, ContextMenuEntry::kEnabled);

  module->AddSymbols({});
  EXPECT_EQ(module->GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kDebugSymbols);
  context_menu =
      FlattenContextMenuWithGroupingAndCheckOrder(view_.GetContextMenuWithGrouping(0, {kIndex}));
  CheckSingleAction(context_menu, kMenuActionLoadSymbols, ContextMenuEntry::kDisabled);
}

TEST_F(ModulesDataViewTest, ContextMenuActionsAreInvoked) {
  AddModulesByIndices({0});
  FlattenContextMenu context_menu =
      FlattenContextMenuWithGroupingAndCheckOrder(view_.GetContextMenuWithGrouping(0, {0}));
  ASSERT_FALSE(context_menu.empty());

  // Load Symbols
  {
    const int load_symbols_index = GetActionIndexOnMenu(context_menu, kMenuActionLoadSymbols);
    EXPECT_TRUE(load_symbols_index != kInvalidActionIndex);

    EXPECT_CALL(app_, LoadSymbolsManually)
        .Times(1)
        .WillOnce(testing::Return(orbit_base::Future<void>{}));
    view_.OnContextMenu(std::string{kMenuActionLoadSymbols}, load_symbols_index, {0});
  }

  // Copy Selection
  {
    EXPECT_CALL(app_, GetSymbolLoadingStateForModule)
        .WillOnce(testing::Return(SymbolLoadingState::kLoaded));
    std::string expected_clipboard = absl::StrFormat(
        "Symbols\tName\tPath\tAddress Range\tFile Size\n"
        "Loaded\t%s\t%s\t%s\t%s\n",
        kNames[0], kFilePaths[0], GetExpectedDisplayAddressRangeByIndex(0),
        GetExpectedDisplayFileSizeByIndex(0));
    CheckCopySelectionIsInvoked(context_menu, app_, view_, expected_clipboard);
  }

  // Export to CSV
  {
    EXPECT_CALL(app_, GetSymbolLoadingStateForModule)
        .WillOnce(testing::Return(SymbolLoadingState::kLoaded));
    std::string expected_contents =
        absl::StrFormat(R"("Symbols","Name","Path","Address Range","File Size")"
                        "\r\n"
                        R"("Loaded","%s","%s","%s","%s")"
                        "\r\n",
                        kNames[0], kFilePaths[0], GetExpectedDisplayAddressRangeByIndex(0),
                        GetExpectedDisplayFileSizeByIndex(0));
    CheckExportToCsvIsInvoked(context_menu, app_, view_, expected_contents);
  }
}

TEST_F(ModulesDataViewTest, LoadModuleOnDoubleClick) {
  EXPECT_CALL(app_, LoadSymbolsManually)
      .Times(1)
      .WillOnce(testing::Return(orbit_base::Future<void>{}));

  AddModulesByIndices({0});
  view_.OnDoubleClicked(0);
}

TEST_F(ModulesDataViewTest, FilteringShowsRightResults) {
  AddModulesByIndices({0, 1, 2});

  // Filtering by path with single token
  {
    view_.OnFilter("abc");
    EXPECT_EQ(view_.GetNumElements(), 2);
    EXPECT_THAT((std::array{view_.GetValue(0, kColumnPath), view_.GetValue(1, kColumnPath)}),
                testing::UnorderedElementsAre(kFilePaths[0], kFilePaths[1]));
  }

  // Filtering by path with multiple tokens separated by " ".
  {
    view_.OnFilter("abc usr");
    EXPECT_EQ(view_.GetNumElements(), 1);
    EXPECT_EQ(view_.GetValue(0, kColumnPath), kFilePaths[0]);
  }

  // Fileter by address range
  {
    view_.OnFilter("1100");
    EXPECT_EQ(view_.GetNumElements(), 1);
    EXPECT_EQ(view_.GetValue(0, kColumnAddressRange), GetExpectedDisplayAddressRangeByIndex(0));
  }

  // Filter by path and address range
  {
    view_.OnFilter("abc 1100");
    EXPECT_EQ(view_.GetNumElements(), 1);
    EXPECT_EQ(view_.GetValue(0, kColumnPath), kFilePaths[0]);
    EXPECT_EQ(view_.GetValue(0, kColumnAddressRange), GetExpectedDisplayAddressRangeByIndex(0));
  }

  // No matching result
  {
    view_.OnFilter("abcdefg");
    EXPECT_EQ(view_.GetNumElements(), 0);
  }
}

TEST_F(ModulesDataViewTest, ColumnSortingShowsRightResults) {
  AddModulesByIndices({0, 1, 2});

  using ViewRowEntry = std::array<std::string, kNumColumns>;
  std::vector<ViewRowEntry> view_entries;
  for (const ModuleInMemory& module_in_memory : modules_in_memory_) {
    const ModuleData* module =
        module_manager_.GetMutableModuleByModuleIdentifier(module_in_memory.module_id());

    ViewRowEntry entry;
    entry[kColumnName] = module->name();
    entry[kColumnPath] = module->file_path();
    entry[kColumnFileSize] = orbit_display_formats::GetDisplaySize(module->file_size());
    entry[kColumnAddressRange] =
        absl::StrFormat("[%016x - %016x]", module_in_memory.start(), module_in_memory.end());

    view_entries.push_back(entry);
  }

  auto sort_and_verify = [&](int column_index, orbit_data_views::DataView::SortingOrder order) {
    view_.OnSort(column_index, order);

    std::sort(view_entries.begin(), view_entries.end(),
              [column_index, order](const auto& lhs, const auto& rhs) {
                switch (order) {
                  case orbit_data_views::DataView::SortingOrder::kAscending:
                    return lhs[column_index] < rhs[column_index];
                  case orbit_data_views::DataView::SortingOrder::kDescending:
                    return lhs[column_index] > rhs[column_index];
                  default:
                    ORBIT_UNREACHABLE();
                }
              });

    for (size_t index = 0; index < view_entries.size(); ++index) {
      EXPECT_EQ(view_.GetValue(index, kColumnName), view_entries[index][kColumnName]);
      EXPECT_EQ(view_.GetValue(index, kColumnPath), view_entries[index][kColumnPath]);
      EXPECT_EQ(view_.GetValue(index, kColumnFileSize), view_entries[index][kColumnFileSize]);
      EXPECT_EQ(view_.GetValue(index, kColumnAddressRange),
                view_entries[index][kColumnAddressRange]);
    }
  };

  // Sort by name ascending
  { sort_and_verify(kColumnName, orbit_data_views::DataView::SortingOrder::kAscending); }

  // Sort by name descending
  { sort_and_verify(kColumnName, orbit_data_views::DataView::SortingOrder::kDescending); }

  // Sort by path ascending
  { sort_and_verify(kColumnPath, orbit_data_views::DataView::SortingOrder::kAscending); }

  // Sort by path descending
  { sort_and_verify(kColumnPath, orbit_data_views::DataView::SortingOrder::kDescending); }

  // Sort by file size ascending
  { sort_and_verify(kColumnFileSize, orbit_data_views::DataView::SortingOrder::kAscending); }

  // Sort by file size descending
  { sort_and_verify(kColumnFileSize, orbit_data_views::DataView::SortingOrder::kDescending); }

  // Sort by address range ascending
  { sort_and_verify(kColumnAddressRange, orbit_data_views::DataView::SortingOrder::kAscending); }

  // Sort by address range descending
  { sort_and_verify(kColumnAddressRange, orbit_data_views::DataView::SortingOrder::kDescending); }
}

TEST_F(ModulesDataViewTest, SymbolLoadingColumnContent) {
  constexpr int kIndex = 0;
  AddModulesByIndices({kIndex});
  const ModuleData* module =
      module_manager_.GetModuleByModuleIdentifier(modules_in_memory_[kIndex].module_id());

  auto get_content_for = [this, module](SymbolLoadingState state) -> std::string {
    EXPECT_CALL(app_, GetSymbolLoadingStateForModule(module)).WillOnce(testing::Return(state));
    return view_.GetValue(0, kColumnSymbols);
  };

  EXPECT_EQ(get_content_for(SymbolLoadingState::kUnknown), "");
  EXPECT_EQ(get_content_for(SymbolLoadingState::kDisabled), "Disabled");
  EXPECT_EQ(get_content_for(SymbolLoadingState::kDownloading), "Downloading...");
  EXPECT_EQ(get_content_for(SymbolLoadingState::kError), "Error");
  EXPECT_EQ(get_content_for(SymbolLoadingState::kLoading), "Loading...");
  EXPECT_EQ(get_content_for(SymbolLoadingState::kLoaded), "Loaded");
}

TEST_F(ModulesDataViewTest, SymbolLoadingColor) {
  constexpr int kIndex = 0;
  AddModulesByIndices({kIndex});
  const ModuleData* module =
      module_manager_.GetModuleByModuleIdentifier(modules_in_memory_[kIndex].module_id());

  auto check_color_correct_for = [this, module](SymbolLoadingState state) {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    bool default_color = !state.GetDisplayColor(red, green, blue);

    EXPECT_CALL(app_, GetSymbolLoadingStateForModule(module)).WillOnce(testing::Return(state));
    uint8_t view_red = 0;
    uint8_t view_green = 0;
    uint8_t view_blue = 0;
    bool view_default_color =
        !view_.GetDisplayColor(0, kColumnSymbols, view_red, view_green, view_blue);
    EXPECT_EQ(default_color, view_default_color);
    EXPECT_EQ(red, view_red);
    EXPECT_EQ(green, view_green);
    EXPECT_EQ(blue, view_blue);
  };

  check_color_correct_for(SymbolLoadingState::kUnknown);
  check_color_correct_for(SymbolLoadingState::kDisabled);
  check_color_correct_for(SymbolLoadingState::kDownloading);
  check_color_correct_for(SymbolLoadingState::kError);
  check_color_correct_for(SymbolLoadingState::kLoading);
  check_color_correct_for(SymbolLoadingState::kLoaded);
}

}  // namespace orbit_data_views