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

void CheckSingleAction(const std::vector<std::string>& context_menu, std::string_view action,
                       ContextMenuEntry menu_entry) {
  switch (menu_entry) {
    case ContextMenuEntry::kEnabled:
      EXPECT_THAT(context_menu, testing::Contains(action));
      return;
    case ContextMenuEntry::kDisabled:
      EXPECT_THAT(context_menu, testing::Not(testing::Contains(action)));
      return;
    default:
      UNREACHABLE();
  }
}

void CheckCopySelectionIsInvoked(const std::vector<std::string>& context_menu,
                                 const MockAppInterface& app, DataView& view,
                                 const std::string& expected_clipboard) {
  const auto copy_selection_index =
      std::find(context_menu.begin(), context_menu.end(), kMenuActionCopySelection) -
      context_menu.begin();
  ASSERT_LT(copy_selection_index, context_menu.size());

  std::string clipboard;
  EXPECT_CALL(app, SetClipboard).Times(1).WillOnce(testing::SaveArg<0>(&clipboard));
  view.OnContextMenu(std::string{kMenuActionCopySelection}, static_cast<int>(copy_selection_index),
                     {0});
  EXPECT_EQ(clipboard, expected_clipboard);
}

void CheckExportToCsvIsInvoked(const std::vector<std::string>& context_menu,
                               const MockAppInterface& app, DataView& view,
                               const std::string& expected_contents, std::string_view action_name) {
  const auto action_index =
      std::find(context_menu.begin(), context_menu.end(), action_name) - context_menu.begin();
  ASSERT_LT(action_index, context_menu.size());

  ErrorMessageOr<orbit_base::TemporaryFile> temporary_file_or_error =
      orbit_base::TemporaryFile::Create();
  ASSERT_THAT(temporary_file_or_error, orbit_test_utils::HasNoError());
  const std::filesystem::path temporary_file_path = temporary_file_or_error.value().file_path();

  // We actually only need a temporary file path, so let's call `CloseAndRemove` and reuse the
  // filepath. The TemporaryFile instance will still take care of deleting our new file when it
  // gets out of scope.
  temporary_file_or_error.value().CloseAndRemove();

  EXPECT_CALL(app, GetSaveFile).Times(1).WillOnce(testing::Return(temporary_file_path.string()));
  view.OnContextMenu(std::string{action_name}, static_cast<int>(action_index), {0});

  ErrorMessageOr<std::string> contents_or_error = orbit_base::ReadFileToString(temporary_file_path);
  ASSERT_THAT(contents_or_error, orbit_test_utils::HasNoError());

  EXPECT_EQ(contents_or_error.value(), expected_contents);
}

std::vector<std::string> FlattenContextMenuWithGrouping(
    const std::vector<std::vector<std::string>>& menu_with_grouping) {
  std::vector<std::string> menu;
  for (const std::vector<std::string>& action_group : menu_with_grouping) {
    orbit_base::Append(menu, action_group);
  }
  return menu;
}

}  // namespace orbit_data_views