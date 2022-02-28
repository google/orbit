// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViewTestUtils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "OrbitBase/Append.h"
#include "OrbitBase/File.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/TemporaryFile.h"
#include "TestUtils/TestUtils.h"

namespace orbit_data_views {

int GetActionIndexOnMenu(const FlattenContextMenu& context_menu, std::string_view action) {
  auto matcher = [&action](std::pair<std::string, bool> action_name_and_availability) {
    return action_name_and_availability.first == std::string{action};
  };

  const auto menu_index = static_cast<int>(
      std::find_if(context_menu.begin(), context_menu.end(), matcher) - context_menu.begin());

  if (menu_index == static_cast<int>(context_menu.size())) return kInvalidActionIndex;

  return menu_index;
}

void CheckSingleAction(const FlattenContextMenu& context_menu, std::string_view action,
                       ContextMenuEntry menu_entry) {
  const int action_index = GetActionIndexOnMenu(context_menu, action);
  EXPECT_TRUE(action_index != kInvalidActionIndex);

  switch (menu_entry) {
    case ContextMenuEntry::kEnabled:
      EXPECT_TRUE(context_menu[action_index].second);
      break;
    case ContextMenuEntry::kDisabled:
      EXPECT_FALSE(context_menu[action_index].second);
  }
}

void CheckCopySelectionIsInvoked(const FlattenContextMenu& context_menu,
                                 const MockAppInterface& app, DataView& view,
                                 const std::string& expected_clipboard) {
  const int action_index = GetActionIndexOnMenu(context_menu, kMenuActionCopySelection);
  EXPECT_TRUE(action_index != kInvalidActionIndex);

  std::string clipboard;
  EXPECT_CALL(app, SetClipboard).Times(1).WillOnce(testing::SaveArg<0>(&clipboard));
  view.OnContextMenu(std::string{kMenuActionCopySelection}, action_index, {0});
  EXPECT_EQ(clipboard, expected_clipboard);
}

void CheckExportToCsvIsInvoked(const FlattenContextMenu& context_menu, const MockAppInterface& app,
                               DataView& view, const std::string& expected_contents,
                               std::string_view action_name) {
  const int action_index = GetActionIndexOnMenu(context_menu, action_name);
  EXPECT_TRUE(action_index != kInvalidActionIndex);

  ErrorMessageOr<orbit_base::TemporaryFile> temporary_file_or_error =
      orbit_base::TemporaryFile::Create();
  ASSERT_THAT(temporary_file_or_error, orbit_test_utils::HasNoError());
  const std::filesystem::path temporary_file_path = temporary_file_or_error.value().file_path();

  // We actually only need a temporary file path, so let's call `CloseAndRemove` and reuse the
  // filepath. The TemporaryFile instance will still take care of deleting our new file when it
  // gets out of scope.
  temporary_file_or_error.value().CloseAndRemove();

  EXPECT_CALL(app, GetSaveFile).Times(1).WillOnce(testing::Return(temporary_file_path.string()));
  view.OnContextMenu(std::string{action_name}, action_index, {0});

  ErrorMessageOr<std::string> contents_or_error = orbit_base::ReadFileToString(temporary_file_path);
  ASSERT_THAT(contents_or_error, orbit_test_utils::HasNoError());

  EXPECT_EQ(contents_or_error.value(), expected_contents);
}

void CheckContextMenuOrder(const FlattenContextMenu& context_menu) {
  const std::vector<std::string_view> ordered_action_names = {
      /* Hooking related actions */
      kMenuActionLoadSymbols, kMenuActionSelect, kMenuActionUnselect, kMenuActionEnableFrameTrack,
      kMenuActionDisableFrameTrack, kMenuActionVerifyFramePointers,
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
    const std::vector<ActionGroup>& menu_with_grouping) {
  FlattenContextMenu menu;
  for (const ActionGroup& action_group : menu_with_grouping) {
    for (const auto& action_name_and_availability : action_group) {
      menu.push_back(action_name_and_availability);
    }
  }

  CheckContextMenuOrder(menu);
  return menu;
}

}  // namespace orbit_data_views