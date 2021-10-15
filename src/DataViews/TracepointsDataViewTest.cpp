// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "DataViews/DataView.h"
#include "DataViews/TracepointsDataView.h"
#include "MockAppInterface.h"
#include "OrbitBase/File.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/TemporaryFile.h"
#include "TestUtils/TestUtils.h"
#include "tracepoint.pb.h"

using orbit_grpc_protos::TracepointInfo;

namespace {

constexpr int kColumnSelected = 0;
constexpr int kColumnCategory = 1;
constexpr int kColumnName = 2;
constexpr int kNumColumns = 3;

constexpr char kTracepointSelected[] = "X";
constexpr char kTracepointUnselected[] = "-";

constexpr size_t kNumTracepoints = 3;
const std::array<std::string, kNumTracepoints> kTracepointCategories{"syscalls", "syscalls",
                                                                     "sched"};
const std::array<std::string, kNumTracepoints> kTracepointNames{"sys_enter_kill", "sys_exit_kill",
                                                                "sched_wait_task"};

class TracepointsDataViewTest : public testing::Test {
 public:
  explicit TracepointsDataViewTest() : view_{&app_} {
    for (size_t i = 0; i < kNumTracepoints; i++) {
      TracepointInfo tracepoint;
      tracepoint.set_category(kTracepointCategories[i]);
      tracepoint.set_name(kTracepointNames[i]);

      tracepoints_.push_back(tracepoint);
    }
  }

  void SetTracepointsByIndices(const std::vector<size_t>& indices) {
    std::vector<TracepointInfo> tracepoints_to_add;
    for (size_t index : indices) {
      CHECK(index < kNumTracepoints);
      tracepoints_to_add.push_back(tracepoints_[index]);
    }

    view_.SetTracepoints(tracepoints_to_add);
  }

 protected:
  orbit_data_views::MockAppInterface app_;
  orbit_data_views::TracepointsDataView view_;
  std::vector<TracepointInfo> tracepoints_;
};

}  // namespace

TEST_F(TracepointsDataViewTest, ColumnHeadersNotEmpty) {
  EXPECT_EQ(view_.GetColumns().size(), kNumColumns);
  for (const auto& column : view_.GetColumns()) {
    EXPECT_FALSE(column.header.empty());
  }
}

TEST_F(TracepointsDataViewTest, HasValidDefaultSortingColumn) {
  EXPECT_GE(view_.GetDefaultSortingColumn(), kColumnCategory);
  EXPECT_LT(view_.GetDefaultSortingColumn(), view_.GetColumns().size());
}

TEST_F(TracepointsDataViewTest, ColumnValuesAreCorrect) {
  SetTracepointsByIndices({0});
  bool tracepoint_selected = false;
  EXPECT_CALL(app_, IsTracepointSelected)
      .WillRepeatedly(testing::ReturnPointee(&tracepoint_selected));

  EXPECT_EQ(view_.GetValue(0, kColumnCategory), kTracepointCategories[0]);
  EXPECT_EQ(view_.GetValue(0, kColumnName), kTracepointNames[0]);
  EXPECT_EQ(view_.GetValue(0, kColumnSelected), kTracepointUnselected);

  tracepoint_selected = true;
  EXPECT_EQ(view_.GetValue(0, kColumnSelected), kTracepointSelected);
}

TEST_F(TracepointsDataViewTest, ContextMenuEntriesArePresentCorrectly) {
  std::array<bool, kNumTracepoints> tracepoints_selected{true, true, false};
  auto get_index_from_tracepoint_info =
      [&](const TracepointInfo& tracepoint) -> std::optional<size_t> {
    for (size_t i = 0; i < kNumTracepoints; i++) {
      if (kTracepointNames[i] == tracepoint.name()) return i;
    }
    return std::nullopt;
  };
  EXPECT_CALL(app_, IsTracepointSelected)
      .WillRepeatedly([&](const TracepointInfo& tracepoint) -> bool {
        std::optional<size_t> index = get_index_from_tracepoint_info(tracepoint);
        EXPECT_TRUE(index.has_value());
        return tracepoints_selected.at(index.value());
      });

  auto check_single_action = [](const std::vector<std::string>& context_menu,
                                const std::string& action, bool enable_action) {
    if (enable_action) {
      EXPECT_THAT(context_menu, testing::IsSupersetOf({action}));
    } else {
      EXPECT_THAT(context_menu, testing::Not(testing::Contains(action)));
    }
  };

  auto run_checks = [&](const std::vector<int>& selected_indices) {
    std::vector<std::string> context_menu = view_.GetContextMenu(0, selected_indices);

    // Common actions should always be available.
    check_single_action(context_menu, "Copy Selection", true);
    check_single_action(context_menu, "Export to CSV", true);

    // Unhook action is available if and only if there are selected tracepoints.
    // Hook action is available if and only if there are unselected tracepoints.
    bool enable_unselect = false;
    bool enable_select = false;
    for (size_t index : selected_indices) {
      if (tracepoints_selected[index]) {
        enable_unselect = true;
      } else {
        enable_select = true;
      }
    }
    check_single_action(context_menu, "Unhook", enable_unselect);
    check_single_action(context_menu, "Hook", enable_select);
  };

  SetTracepointsByIndices({0, 1, 2});
  { run_checks({0}); }
  { run_checks({1}); }
  { run_checks({2}); }
  { run_checks({0, 1, 2}); }
}

TEST_F(TracepointsDataViewTest, ContextMenuActionsAreInvoked) {
  bool tracepoint_selected = false;

  EXPECT_CALL(app_, IsTracepointSelected)
      .WillRepeatedly(testing::ReturnPointee(&tracepoint_selected));

  SetTracepointsByIndices({0});
  std::vector<std::string> context_menu = view_.GetContextMenu(0, {0});
  ASSERT_FALSE(context_menu.empty());

  // Copy Selection
  {
    const auto copy_selection_index =
        std::find(context_menu.begin(), context_menu.end(), "Copy Selection") -
        context_menu.begin();
    ASSERT_LT(copy_selection_index, context_menu.size());

    std::string clipboard;
    EXPECT_CALL(app_, SetClipboard).Times(1).WillOnce(testing::SaveArg<0>(&clipboard));
    view_.OnContextMenu("Copy Selection", static_cast<int>(copy_selection_index), {0});
    EXPECT_EQ(clipboard, absl::StrFormat("Selected\tCategory\tName\n"
                                         "%s\t%s\t%s\n",
                                         kTracepointUnselected, kTracepointCategories[0],
                                         kTracepointNames[0]));
  }

  // Export to CSV
  {
    const auto export_to_csv_index =
        std::find(context_menu.begin(), context_menu.end(), "Copy Selection") -
        context_menu.begin();
    ASSERT_LT(export_to_csv_index, context_menu.size());

    ErrorMessageOr<orbit_base::TemporaryFile> temporary_file_or_error =
        orbit_base::TemporaryFile::Create();
    ASSERT_THAT(temporary_file_or_error, orbit_test_utils::HasNoError());
    const std::filesystem::path temporary_file_path = temporary_file_or_error.value().file_path();
    temporary_file_or_error.value().CloseAndRemove();

    EXPECT_CALL(app_, GetSaveFile).Times(1).WillOnce(testing::Return(temporary_file_path.string()));
    view_.OnContextMenu("Export to CSV", static_cast<int>(export_to_csv_index), {0});

    ErrorMessageOr<std::string> contents_or_error =
        orbit_base::ReadFileToString(temporary_file_path);
    ASSERT_THAT(contents_or_error, orbit_test_utils::HasNoError());

    EXPECT_EQ(
        contents_or_error.value(),
        absl::StrFormat(R"("Selected","Category","Name")"
                        "\r\n"
                        R"("%s","%s","%s")"
                        "\r\n",
                        kTracepointUnselected, kTracepointCategories[0], kTracepointNames[0]));
  }

  // Hook
  {
    const auto hook_index =
        std::find(context_menu.begin(), context_menu.end(), "Hook") - context_menu.begin();
    ASSERT_LT(hook_index, context_menu.size());

    EXPECT_CALL(app_, SelectTracepoint).Times(1).WillOnce([&](const TracepointInfo& tracepoint) {
      EXPECT_EQ(tracepoint.name(), kTracepointNames[0]);
    });
    view_.OnContextMenu("Hook", static_cast<int>(hook_index), {0});
  }

  tracepoint_selected = true;
  context_menu = view_.GetContextMenu(0, {0});
  ASSERT_FALSE(context_menu.empty());

  // Unhook
  {
    const auto unhook_index =
        std::find(context_menu.begin(), context_menu.end(), "Unhook") - context_menu.begin();
    ASSERT_LT(unhook_index, context_menu.size());

    EXPECT_CALL(app_, DeselectTracepoint).Times(1).WillOnce([&](const TracepointInfo& tracepoint) {
      EXPECT_EQ(tracepoint.name(), kTracepointNames[0]);
    });
    view_.OnContextMenu("Unhook", static_cast<int>(unhook_index), {0});
  }
}

TEST_F(TracepointsDataViewTest, FilteringShowsRightResults) {
  SetTracepointsByIndices({0, 1, 2});

  // Filtering by tracepoint name with single token
  {
    view_.OnFilter("wait");
    EXPECT_EQ(view_.GetNumElements(), 1);
    EXPECT_EQ(view_.GetValue(0, kColumnName), kTracepointNames[2]);
  }

  // Filtering by tracepoint name with multiple tokens separated by " ".
  {
    view_.OnFilter("sys kill");
    EXPECT_EQ(view_.GetNumElements(), 2);
    EXPECT_THAT((std::array{view_.GetValue(0, kColumnName), view_.GetValue(1, kColumnName)}),
                testing::UnorderedElementsAre(kTracepointNames[0], kTracepointNames[1]));
  }

  // No matching result
  {
    view_.OnFilter("abcdefg");
    EXPECT_EQ(view_.GetNumElements(), 0);
  }
}

TEST_F(TracepointsDataViewTest, ColumnSortingShowsRightResults) {
  SetTracepointsByIndices({0, 1, 2});

  using ViewRowEntry = std::array<std::string, kNumColumns>;
  std::vector<ViewRowEntry> view_entries;
  for (size_t i = 0; i < view_.GetNumElements(); i++) {
    ViewRowEntry entry;
    entry[kColumnCategory] = kTracepointCategories[i];
    entry[kColumnName] = kTracepointNames[i];
    view_entries.push_back(entry);
  }

  auto sort_and_verify = [&](int column, orbit_data_views::DataView::SortingOrder order) {
    view_.OnSort(column, order);
    // Columns sorted by display values (i.e., string).
    std::sort(view_entries.begin(), view_entries.end(),
              [column, order](const ViewRowEntry& lhs, const ViewRowEntry& rhs) {
                switch (order) {
                  case orbit_data_views::DataView::SortingOrder::kAscending:
                    return lhs[column] < rhs[column];
                  case orbit_data_views::DataView::SortingOrder::kDescending:
                    return lhs[column] > rhs[column];
                  default:
                    UNREACHABLE();
                }
              });

    for (size_t index = 0; index < view_entries.size(); ++index) {
      for (int column = kColumnCategory; column < kNumColumns; ++column) {
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