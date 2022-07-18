"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""
import logging
import time

from core.common_controls import Table
from core.orbit_e2e import E2ETestCase, find_control
from pywinauto.keyboard import send_keys
from typing import Sequence, Set


class VerifyHelloGgpBottomUpContents(E2ETestCase):
    """
    Verify that the first rows of the bottom-up view are in line with hello_ggp.
    In particular, we expect one of the first three rows to be "ioctl", the first child of the "ioctl" row to be "drm*",
    and a reasonable total number of rows.
    """

    def _execute(self):
        self.find_control("TabItem", "Bottom-Up").click_input()
        logging.info('Switched to Bottom-Up tab')

        tree_view_table = Table(find_control(self.find_control('Group', 'bottomUpTab'), 'Tree'))

        logging.info('Counting rows of the bottom-up view...')
        if tree_view_table.get_row_count() < 10:
            raise RuntimeError('Less than 10 rows in the bottom-up view')

        ioctl_row = -1
        for row in range(3):
            row_tree_item = tree_view_table.get_item_at(row, 0)
            if row_tree_item.window_text().endswith('ioctl'):
                ioctl_row = row
        if ioctl_row == -1:
            raise RuntimeError('"ioctl" is not among the first three item of the bottom-up view')
        logging.info('Verified that "ioctl" is one of the first three items')

        ioctl_row_tree_item = tree_view_table.get_item_at(ioctl_row, 0)
        ioctl_row_tree_item.double_click_input()
        logging.info('Expanded the "ioctl" item')

        first_ioctl_child_tree_item = tree_view_table.get_item_at(ioctl_row + 1, 0)
        if not first_ioctl_child_tree_item.window_text().startswith('drm'):
            raise RuntimeError(
                'First child of the "ioctl" item in the bottom-up view doesn\'t start with "drm"')
        logging.info('Verified that first child of the "ioctl" item starts with "drm"')


class VerifyBottomUpContentForLoadedCapture(E2ETestCase):
    """
    Verify the content of the bottom-up view for the capture loaded from "OrbitTest_1-72.orbit".
    In particular, we verify the number of rows and the content of the first rows when the tree is expanded in a few
    different ways. Then we search for a function and verify that the tree is expanded to show it.
    """

    def _verify_row_count(self, tree_view_table: Table, expected_rows: int):
        self.expect_eq(tree_view_table.get_row_count(), expected_rows,
                       "Bottom-up view has {} rows".format(expected_rows))

    def _verify_first_rows(self, tree_view_table: Table, expectations: Sequence[Sequence[str]],
                           hidden_columns: Set[int]):
        for i in range(len(expectations)):
            for j in range(len(expectations[i])):
                if j in hidden_columns:
                    continue
                expectation = expectations[i][j]
                self.expect_eq(
                    tree_view_table.get_item_at(i, j).window_text(), expectation,
                    "Bottom-up view's cell ({}, {}) is '{}'".format(i, j, expectation))

    def _switch_to_tab(self, selection_tab: bool = False):
        if not selection_tab:
            self.find_control('TabItem', 'Bottom-Up').click_input()
            logging.info("Switched to 'Bottom-Up' tab")
            return self.find_control('Group', 'bottomUpTab')
        else:
            self.find_control('TabItem', 'Bottom-Up (selection)').click_input()
            logging.info("Switched to 'Bottom-Up (selection)' tab")
            return self.find_control('Group', 'selectionBottomUpTab')

    def _collapse_tree(self, tree_view_table: Table):
        tree_view_table.get_item_at(0, 0).click_input(button='right')
        self.find_context_menu_item("Collapse all").click_input()

    HIDDEN_COLUMNS = {2}

    def _verify_columns(self, tree_view_table):
        # Note that one column is hidden.
        self.expect_eq(tree_view_table.get_column_count(), 6, "Bottom-up view has 6 columns")

    EXPECTED_LEAF_COUNT = 132

    def _verify_rows_when_tree_collapsed(self, tree_view_table):
        self._verify_row_count(tree_view_table, self.EXPECTED_LEAF_COUNT)

        expectations = [
            ["clock_gettime", "61.66% (976)", "", "", "[vdso]", "0x7fff627d1a00"],
            [
                "std::__1::chrono::system_clock::now()", "8.59% (136)", "", "", "libc++.so.1.0",
                "0x7fe86ff978e0"
            ],
        ]
        self._verify_first_rows(tree_view_table, expectations, self.HIDDEN_COLUMNS)
        logging.info("Verified content of bottom-up view when all leaves are collapsed")

    def _verify_rows_when_node_expanded(self, tree_view_table):
        logging.info("Expanding a node of the bottom-up view")
        tree_view_table.get_item_at(0, 0).double_click_input()

        expected_first_leaf_child = 1
        self._verify_row_count(tree_view_table,
                               self.EXPECTED_LEAF_COUNT + expected_first_leaf_child)

        expectations = [
            ["clock_gettime", "61.66% (976)", "", "", "[vdso]", "0x7fff627d1a00"],
            ["__clock_gettime", "61.66% (976)", "", "100.00%", "libc-2.24.so", "0x7fe86f002940"],
            [
                "std::__1::chrono::system_clock::now()", "8.59% (136)", "", "", "libc++.so.1.0",
                "0x7fe86ff978e0"
            ],
        ]
        self._verify_first_rows(tree_view_table, expectations, self.HIDDEN_COLUMNS)
        logging.info("Verified content of bottom-up view when one node is expanded")

    def _verify_rows_when_node_recursively_expanded(self, tree_view_table):
        logging.info("Recursively expanding a node of the bottom-up view")
        tree_view_table.get_item_at(0, 0).click_input(button='right')
        self.find_context_menu_item('Expand recursively\tALT+Click').click_input()

        expected_first_leaf_descendant_count = 50
        self._verify_row_count(tree_view_table,
                               self.EXPECTED_LEAF_COUNT + expected_first_leaf_descendant_count)

        expectations = [
            ["clock_gettime", "61.66% (976)", "", "", "[vdso]", "0x7fff627d1a00"],
            ["__clock_gettime", "61.66% (976)", "", "100.00%", "libc-2.24.so", "0x7fe86f002940"],
            [
                "std::__1::chrono::system_clock::now()", "61.53% (974)", "", "99.80%",
                "libc++.so.1.0", "0x7fe86ff978e0"
            ],
            [
                "OrbitTestImpl::BusyWork(unsigned long)", "61.53% (974)", "", "100.00%",
                "OrbitTest", "0x55a44717b2c0"
            ],
        ]
        self._verify_first_rows(tree_view_table, expectations, self.HIDDEN_COLUMNS)
        logging.info("Verified content of bottom-up view when one node is recursively expanded")

    def _verify_rows_when_tree_expanded(self, tree_view_table):
        logging.info("Expanding the entire bottom-up view")
        tree_view_table.get_item_at(0, 0).click_input(button='right')
        self.find_context_menu_item('Expand all').click_input()

        self._verify_row_count(tree_view_table, 1914)

        expectations = [
            ["clock_gettime", "61.66% (976)", "", "", "[vdso]", "0x7fff627d1a00"],
            ["__clock_gettime", "61.66% (976)", "", "100.00%", "libc-2.24.so", "0x7fe86f002940"],
            [
                "std::__1::chrono::system_clock::now()", "61.53% (974)", "", "99.80%",
                "libc++.so.1.0", "0x7fe86ff978e0"
            ],
            [
                "OrbitTestImpl::BusyWork(unsigned long)", "61.53% (974)", "", "100.00%",
                "OrbitTest", "0x55a44717b2c0"
            ],
        ]
        self._verify_first_rows(tree_view_table, expectations, self.HIDDEN_COLUMNS)
        logging.info("Verified content of bottom-up view when it is completely expanded")

    def _verify_rows_on_search(self, tab, tree_view_table):
        search_term = 'eventfd_write'
        logging.info("Searching bottom-up view for '{}'".format(search_term))
        search_bar = find_control(parent=tab, control_type='Edit', name='filter')
        search_bar.set_focus()
        send_keys(search_term)
        time.sleep(1)  # The bottom-up view waits for the user to stop typing before searching.
        self._verify_row_count(tree_view_table, self.EXPECTED_LEAF_COUNT + 1)

        search_item_count = 0
        for i in range(tree_view_table.get_row_count()):
            if tree_view_table.get_item_at(i, 0).window_text() == search_term:
                search_item_count += 1
                expectations = [
                    "eventfd_write", "1.77% (28)", "", "100.00%", "libc-2.24.so", "0x7fe86eff5c30"
                ]
                for j in range(len(expectations)):
                    if j in self.HIDDEN_COLUMNS:
                        continue
                    expectation = expectations[j]
                    self.expect_eq(
                        tree_view_table.get_item_at(i, j).window_text(), expectation,
                        "Bottom-up view's cell ({}, {}) is '{}'".format(i, j, expectation))
        self.expect_eq(search_item_count, 1,
                       "Searching bottom-up view for '{}' produces one result".format(search_term))
        logging.info("Verified result of searching bottom-up view for '{}'".format(search_term))

        # Clear the search terms.
        search_bar.set_focus()
        send_keys('^a{BACKSPACE}')

    def _execute(self, selection_tab: bool = False):
        tab = self._switch_to_tab(selection_tab)
        tree_view_table = Table(find_control(tab, 'Tree'))

        self._verify_columns(tree_view_table)
        self._verify_rows_when_tree_collapsed(tree_view_table)
        self._verify_rows_when_node_expanded(tree_view_table)
        self._collapse_tree(tree_view_table)
        self._verify_rows_when_node_recursively_expanded(tree_view_table)
        self._collapse_tree(tree_view_table)
        self._verify_rows_when_tree_expanded(tree_view_table)
        self._collapse_tree(tree_view_table)
        self._verify_rows_on_search(tab, tree_view_table)


class VerifyBottomUpContentForTriangleExe(E2ETestCase):
    """
    Verify that the top function in the bottom-up view (i.e., the function with the highest exclusive count) is
    `absl::GetCurrentTimeNanos()`, and verify its caller and its caller's caller.
    """

    def _execute(self):
        self.find_control("TabItem", "Bottom-Up").click_input()
        logging.info("Switched to the 'Bottom-Up' tab")
        tab = self.find_control('Group', 'bottomUpTab')
        tree_view_table = Table(self.find_control('Tree', parent=tab))

        logging.info(
            "Verifying the function with the highest exclusive count and its callers from the bottom-up view"
        )

        first_row_tree_item = tree_view_table.get_item_at(0, 0)
        expected_function = "absl::GetCurrentTimeNanos()"
        self.expect_eq(first_row_tree_item.window_text(), expected_function,
                       f"Top item of the bottom-up view is '{expected_function}'")

        row_count_before_expansion = tree_view_table.get_row_count()
        first_row_tree_item.double_click_input()
        row_count_after_expansion = tree_view_table.get_row_count()
        self.expect_true(row_count_after_expansion > row_count_before_expansion,
                         f"The '{expected_function}' node has at least one caller")

        first_child_tree_item = tree_view_table.get_item_at(1, 0)
        expected_function = "absl::Now()"
        self.expect_eq(first_child_tree_item.window_text(), expected_function,
                       f"Top child of the top item of the bottom-up view is '{expected_function}'")

        row_count_before_expansion = tree_view_table.get_row_count()
        first_child_tree_item.double_click_input()
        row_count_after_expansion = tree_view_table.get_row_count()
        self.expect_true(row_count_after_expansion > row_count_before_expansion,
                         f"The '{expected_function}' node has at least one caller")

        first_grandchild_tree_item = tree_view_table.get_item_at(2, 0)
        expected_function = "yeti::internal::video_ipc::HostMappedSlotBackedFrameSender::AcquireSlot(absl::Time)"
        self.expect_eq(
            first_grandchild_tree_item.window_text(), expected_function,
            f"Top grandchild of the top item of the bottom-up view is '{expected_function}'")

        logging.info(
            "Verified the function with the highest exclusive count and its callers from the bottom-up view"
        )
