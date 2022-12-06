// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_DATA_VIEW_TEST_UTILS_H_
#define DATA_VIEWS_DATA_VIEW_TEST_UTILS_H_

#include <absl/types/span.h>

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

#include "DataViews/DataView.h"
#include "MockAppInterface.h"
#include "TestUtils/TemporaryFile.h"

namespace orbit_data_views {

using FlattenContextMenu = std::vector<DataView::Action>;

constexpr int kInvalidActionIndex = -1;

[[nodiscard]] int GetActionIndexOnMenu(const FlattenContextMenu& context_menu,
                                       std::string_view action_name);

enum class ContextMenuEntry { kEnabled, kDisabled };

void CheckSingleAction(const FlattenContextMenu& context_menu, std::string_view action_name,
                       ContextMenuEntry menu_entry);

void CheckCopySelectionIsInvoked(const FlattenContextMenu& flatten_context_menu,
                                 const MockAppInterface& app, DataView& view,
                                 std::string_view expected_clipboard);

void CheckExportToCsvIsInvoked(const FlattenContextMenu& context_menu, const MockAppInterface& app,
                               DataView& view, std::string_view expected_contents,
                               std::string_view action_name = kMenuActionExportToCsv);

void CheckContextMenuOrder(const FlattenContextMenu& context_menu);

[[nodiscard]] FlattenContextMenu FlattenContextMenuWithGroupingAndCheckOrder(
    absl::Span<const DataView::ActionGroup> menu_with_grouping);

orbit_test_utils::TemporaryFile GetTemporaryFilePath();

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_DATA_VIEW_TEST_UTILS_H_