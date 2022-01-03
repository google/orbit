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
from typing import Sequence


class VerifyHelloGgpBottomUpContents(E2ETestCase):
    """
    Verify that the first rows of the bottom-up view are in line with hello_ggp.
    In particular, we expect the first row to be "ioctl", the second row to be "drm*",
    and a reasonable total number of rows.
    """

    def _execute(self):
        self.find_control("TabItem", "Bottom-Up").click_input()
        logging.info('Switched to Bottom-Up tab')

        tree_view_table = Table(find_control(self.find_control('Group', 'bottomUpTab'), 'Tree'))

        logging.info('Counting rows of the bottom-up view...')
        if tree_view_table.get_row_count() < 10:
            raise RuntimeError('Less than 10 rows in the bottom-up view')

        first_row_tree_item = tree_view_table.get_item_at(0, 0)
        if first_row_tree_item.window_text() != 'ioctl':
            raise RuntimeError('First item of the bottom-up view is not "ioctl"')
        logging.info('Verified that first item is "ioctl"')

        first_row_tree_item.double_click_input()
        logging.info('Expanded the first item')

        second_row_tree_item = tree_view_table.get_item_at(1, 0)
        if not second_row_tree_item.window_text().startswith('drm'):
            raise RuntimeError('First child of the first item ("ioctl") '
                               'of the bottom-up view doesn\'t start with "drm"')
        logging.info('Verified that first child of the first item starts with "drm"')


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
                           hidden_columns: set[int]):
        for i in range(len(expectations)):
            for j in range(len(expectations[i])):
                if j in hidden_columns:
                    continue
                expectation = expectations[i][j]
                self.expect_eq(
                    tree_view_table.get_item_at(i, j).window_text(), expectation,
                    "Bottom-up view's cell ({}, {}) is '{}'".format(i, j, expectation))

    def _execute(self, selection_tab: bool = False):
        if not selection_tab:
            self.find_control('TabItem', 'Bottom-Up').click_input()
            logging.info("Switched to 'Bottom-Up' tab")
            tab = self.find_control('Group', 'bottomUpTab')
        else:
            self.find_control('TabItem', 'Bottom-Up (selection)').click_input()
            logging.info("Switched to 'Bottom-Up (selection)' tab")
            tab = self.find_control('Group', 'selectionBottomUpTab')

        tree_view_table = Table(find_control(tab, 'Tree'))

        # Note that one column is hidden.
        self.expect_eq(tree_view_table.get_column_count(), 6, "Bottom-up view has 6 columns")
        hidden_columns = {2}

        expected_leaf_count = 132
        self._verify_row_count(tree_view_table, expected_leaf_count)

        expectations = [
            ["clock_gettime", "61.66% (976)", "", "", "[vdso]", "0x7fff627d1a00"],
            [
                "std::__1::chrono::system_clock::now()", "8.59% (136)", "", "", "libc++.so.1.0",
                "0x7fe86ff978e0"
            ],
        ]
        self._verify_first_rows(tree_view_table, expectations, hidden_columns)
        logging.info("Verified content of bottom-up view when all leaves are collapsed")

        logging.info("Expanding a leaf of the bottom-up view")
        tree_view_table.get_item_at(0, 0).double_click_input()
        expected_first_leaf_child = 1
        self._verify_row_count(tree_view_table, expected_leaf_count + expected_first_leaf_child)

        expectations = [
            ["clock_gettime", "61.66% (976)", "", "", "[vdso]", "0x7fff627d1a00"],
            ["__clock_gettime", "61.66% (976)", "", "100.00%", "libc-2.24.so", "0x7fe86f002940"],
            [
                "std::__1::chrono::system_clock::now()", "8.59% (136)", "", "", "libc++.so.1.0",
                "0x7fe86ff978e0"
            ],
        ]
        self._verify_first_rows(tree_view_table, expectations, hidden_columns)
        logging.info("Verified content of bottom-up view when one leaf is expanded")

        logging.info("Recursively expanding a leaf of the bottom-up view")
        tree_view_table.get_item_at(0, 0).click_input(button='right')
        self.find_context_menu_item('Expand recursively').click_input()
        expected_first_leaf_descendant_count = 50
        self._verify_row_count(tree_view_table,
                               expected_leaf_count + expected_first_leaf_descendant_count)

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
        self._verify_first_rows(tree_view_table, expectations, hidden_columns)
        logging.info("Verified content of bottom-up view when one leaf is recursively expanded")

        logging.info("Expanding the entire bottom-up view")
        tree_view_table.get_item_at(0, 0).click_input(button='right')
        self.find_context_menu_item('Expand all').click_input()
        self._verify_row_count(tree_view_table, 1908)

        # Previously we recursively expanded the first node, so the expectations on the first rows are still the same.
        self._verify_first_rows(tree_view_table, expectations, hidden_columns)
        logging.info("Verified content of bottom-up view when it is completely expanded")

        search_term = 'eventfd_write'
        logging.info("Searching bottom-up view for '{}'".format(search_term))
        search_bar = find_control(parent=tab, control_type='Edit', name='filter')
        search_bar.set_focus()
        send_keys(search_term)
        time.sleep(1)  # The bottom-up view waits for the user to stop typing before searching.
        self._verify_row_count(tree_view_table, expected_leaf_count + 1)

        search_item_count = 0
        for i in range(tree_view_table.get_row_count()):
            if tree_view_table.get_item_at(i, 0).window_text() == search_term:
                search_item_count += 1
                expectations = [
                    "eventfd_write", "1.77% (28)", "", "100.00%", "libc-2.24.so", "0x7fe86eff5c30"
                ]
                for j in range(len(expectations)):
                    if j in hidden_columns:
                        continue
                    expectation = expectations[j]
                    self.expect_eq(
                        tree_view_table.get_item_at(i, j).window_text(), expectation,
                        "Bottom-up view's cell ({}, {}) is '{}'".format(i, j, expectation))
        self.expect_eq(search_item_count, 1,
                       "Searching bottom-up view for '{}' produces one result".format(search_term))
        logging.info("Verified result of searching bottom-up view for '{}'".format(search_term))
