// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViewTestUtils.h"

#include <absl/strings/str_split.h>
#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <utility>

#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "TestUtils/TemporaryFile.h"
#include "TestUtils/TestUtils.h"

namespace orbit_data_views {

int GetActionIndexOnMenu(const FlattenContextMenu& context_menu, std::string_view action_name) {
  auto matcher = [&action_name](const DataView::Action& action) {
    return action.name == std::string{action_name};
  };

  const auto menu_index = static_cast<int>(
      std::find_if(context_menu.begin(), context_menu.end(), matcher) - context_menu.begin());

  if (menu_index == static_cast<int>(context_menu.size())) return kInvalidActionIndex;

  return menu_index;
}

void CheckSingleAction(const FlattenContextMenu& context_menu, std::string_view action_name,
                       ContextMenuEntry menu_entry) {
  const int action_index = GetActionIndexOnMenu(context_menu, action_name);
  EXPECT_TRUE(action_index != kInvalidActionIndex);
  const DataView::Action& action = context_menu[action_index];

  switch (menu_entry) {
    case ContextMenuEntry::kEnabled:
      EXPECT_TRUE(action.enabled);
      break;
    case ContextMenuEntry::kDisabled:
      EXPECT_FALSE(action.enabled);
  }
}

void CheckCopySelectionIsInvoked(const FlattenContextMenu& context_menu,
                                 const MockAppInterface& app, DataView& view,
                                 std::string_view expected_clipboard) {
  const int action_index = GetActionIndexOnMenu(context_menu, kMenuActionCopySelection);
  EXPECT_TRUE(action_index != kInvalidActionIndex);

  std::string clipboard;
  EXPECT_CALL(app, SetClipboard).Times(1).WillOnce(testing::SaveArg<0>(&clipboard));
  view.OnContextMenu(std::string{kMenuActionCopySelection}, action_index, {0});
  EXPECT_EQ(clipboard, expected_clipboard);
}

static void ExpectSameLines(const std::string_view& actual, const std::string_view& expected) {
  static const std::string delimeter = "\r\n";

  std::vector<std::string_view> actual_lines = absl::StrSplit(actual, delimeter);
  std::vector<std::string_view> expected_lines = absl::StrSplit(expected, delimeter);
  EXPECT_THAT(actual_lines, testing::UnorderedElementsAreArray(expected_lines));
}

[[nodiscard]] orbit_test_utils::TemporaryFile GetTemporaryFilePath() {
  ErrorMessageOr<orbit_test_utils::TemporaryFile> temporary_file_or_error =
      orbit_test_utils::TemporaryFile::Create();
  EXPECT_THAT(temporary_file_or_error, orbit_test_utils::HasNoError());
  return std::move(temporary_file_or_error.value());
}

void CheckExportToCsvIsInvoked(const FlattenContextMenu& context_menu, const MockAppInterface& app,
                               DataView& view, std::string_view expected_contents,
                               std::string_view action_name) {
  const int action_index = GetActionIndexOnMenu(context_menu, action_name);
  EXPECT_TRUE(action_index != kInvalidActionIndex);

  orbit_test_utils::TemporaryFile temporary_file = GetTemporaryFilePath();

  // We actually only need a temporary file path, so let's call `CloseAndRemove` and reuse the
  // filepath. The TemporaryFile instance will still take care of deleting our new file when it
  // gets out of scope.
  temporary_file.CloseAndRemove();

  EXPECT_CALL(app, GetSaveFile)
      .Times(1)
      .WillOnce(testing::Return(temporary_file.file_path().string()));
  view.OnContextMenu(std::string{action_name}, action_index, {0});

  ErrorMessageOr<std::string> contents_or_error =
      orbit_base::ReadFileToString(temporary_file.file_path());
  ASSERT_THAT(contents_or_error, orbit_test_utils::HasNoError());

  ExpectSameLines(contents_or_error.value(), expected_contents);
}

void CheckContextMenuOrder(const FlattenContextMenu& context_menu) {
  const std::vector<std::string_view> ordered_action_names = {
      /* Hooking related actions */
      kMenuActionLoadSymbols, kMenuActionSelect, kMenuActionUnselect, kMenuActionEnableFrameTrack,
      kMenuActionDisableFrameTrack,
      /* Disassembly & source code related actions */
      kMenuActionDisassembly, kMenuActionSourceCode,
      /* Navigating related actions */
      kMenuActionAddIterator, kMenuActionJumpToFirst, kMenuActionJumpToLast, kMenuActionJumpToMin,
      kMenuActionJumpToMax,
      /* Preset related actions */
      kMenuActionLoadPreset, kMenuActionDeletePreset, kMenuActionShowInExplorer,
      /* Exporting related actions */
      kMenuActionCopySelection, kMenuActionExportToCsv, kMenuActionExportEventsToCsv};

  std::vector<int> visible_action_indices;
  for (auto action_name : ordered_action_names) {
    const int action_index = GetActionIndexOnMenu(context_menu, action_name);
    if (action_index == kInvalidActionIndex) continue;
    visible_action_indices.push_back(action_index);
  }

  EXPECT_TRUE(is_sorted(visible_action_indices.begin(), visible_action_indices.end()));
}

FlattenContextMenu FlattenContextMenuWithGroupingAndCheckOrder(
    absl::Span<const DataView::ActionGroup> menu_with_grouping) {
  FlattenContextMenu menu;
  for (const DataView::ActionGroup& action_group : menu_with_grouping) {
    for (const auto& action : action_group) menu.push_back(action);
  }

  CheckContextMenuOrder(menu);
  return menu;
}

}  // namespace orbit_data_views