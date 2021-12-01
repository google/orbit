// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_DATA_VIEW_TEST_UTILS_H_
#define DATA_VIEWS_DATA_VIEW_TEST_UTILS_H_

#include <string>
#include <vector>

#include "DataViews/DataView.h"
#include "MockAppInterface.h"

namespace orbit_data_views {

enum class ContextMenuEntry { kEnabled, kDisabled };

void CheckSingleAction(const std::vector<std::string>& context_menu, std::string_view action,
                       ContextMenuEntry menu_entry);

void CheckCopySelectionIsInvoked(const std::vector<std::string>& context_menu,
                                 const MockAppInterface& app, DataView& view,
                                 const std::string& expected_clipboard);

void CheckExportToCsvIsInvoked(const std::vector<std::string>& context_menu,
                               const MockAppInterface& app, DataView& view,
                               const std::string& expected_contents,
                               std::string_view action_name = kMenuActionExportToCsv);

std::vector<std::string> FlattenContextMenuWithGrouping(
    const std::vector<std::vector<std::string>>& menu_with_grouping);

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_DATA_VIEW_TEST_UTILS_H_