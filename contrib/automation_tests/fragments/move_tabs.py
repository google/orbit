"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging

from core.orbit_e2e import E2ETestCase, find_control


class MoveFunctionsTab(E2ETestCase):
    def right_click_move_context(self, item):
        item.click_input(button='right')
        context_menu = self.suite.application.window(best_match="TabBarContextMenu")
        find_control(context_menu, "MenuItem", name_contains="Move").click_input(button='left')

    def _execute(self):
        wnd = self.suite.top_window()

        # Find "Functions" tab and left and right tab bar
        tab_item = find_control(wnd, "TabItem", name="Functions")
        right_tab_bar = find_control(
            find_control(wnd, "Group", name="RightTabWidget"),
            "Tab", recurse=False)
        left_tab_bar = find_control(
            find_control(wnd, "Group", name="MainTabWidget"),
            "Tab", recurse=False)

        # Init tests
        left_tab_count = left_tab_bar.control_count()
        right_tab_count = right_tab_bar.control_count()

        tab_parent = tab_item.parent()
        self.expect_eq(tab_parent, right_tab_bar, "Functions tab is initialized in the right pane")

        # Move "Functions" tab to the left pane, check no. of tabs and if the tab is enabled
        logging.info('Moving "Functions" tab to the left pane (current tab count: %d)', right_tab_count)
        self.right_click_move_context(tab_item)
        self.expect_eq(right_tab_bar.control_count(), right_tab_count - 1, "1 tab removed from right pane")
        self.expect_eq(left_tab_bar.control_count(), left_tab_count + 1, "1 tab added to the left pane")

        tab_item = find_control(wnd, "TabItem", name="Functions")
        self.expect_eq(tab_item.parent(), left_tab_bar, "Functions tab is parented under the left pane")
        self.expect_true(find_control(wnd, "Group", auto_id_leaf="FunctionsTab").is_visible(),
                         "Functions tab is visible")

        # Move back, check no. of tabs
        logging.info('Moving "Functions" tab back to the right pane')
        self.right_click_move_context(tab_item)
        self.expect_eq(right_tab_bar.control_count(), right_tab_count, "1 tab removed from left pane")
        self.expect_eq(left_tab_bar.control_count(), left_tab_count, "1 tab added to the right pane")

        tab_item = find_control(wnd, "TabItem", name="Functions")
        self.expect_eq(tab_item.parent(), right_tab_bar, "Functions tab is parented under the right pane")
        self.expect_true(find_control(wnd, "Group", auto_id_leaf="FunctionsTab").is_visible(),
                         "Functions tab is visible")
