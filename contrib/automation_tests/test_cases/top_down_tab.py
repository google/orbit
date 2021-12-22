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


class VerifyHelloGgpTopDownContents(E2ETestCase):
    """
    Verify that the first rows of the top-down view are in line with hello_ggp.
    In particular, we expect the first row to be "hello_* (all threads)", with children "*clone" and "_start",
    and the second row to be "hello_*]" or "Ggp*]".
    """

    def _execute(self):
        self.find_control("TabItem", "Top-Down").click_input()
        logging.info('Switched to Top-Down tab')

        tree_view_table = Table(find_control(self.find_control('Group', 'topDownTab'), 'Tree'))

        logging.info('Counting rows of the top-down view...')
        row_count_before_expansion = tree_view_table.get_row_count()

        if row_count_before_expansion < 3:
            raise RuntimeError('Less than 3 rows in the top-down view')

        first_row_tree_item = tree_view_table.get_item_at(0, 0)
        if (not first_row_tree_item.window_text().startswith('hello_') or
                not first_row_tree_item.window_text().endswith(' (all threads)')):
            raise RuntimeError('First item of the top-down view is not "hello_* (all threads)"')
        logging.info('Verified that first item is "hello_* (all threads)"')

        second_row_tree_item = tree_view_table.get_item_at(1, 0)
        if ((not second_row_tree_item.window_text().startswith('hello_') and
             not second_row_tree_item.window_text().startswith('Ggp')) or
                not second_row_tree_item.window_text().endswith(']')):
            raise RuntimeError('Second item of the top-down view is not "hello_*]" nor "Ggp*]"')
        logging.info('Verified that second item is "hello_*]" or "Ggp*]"')

        first_row_tree_item.double_click_input()
        logging.info('Expanded the first item')

        logging.info('Re-counting rows of the top-down view...')
        row_count_after_expansion = tree_view_table.get_row_count()

        if row_count_after_expansion < row_count_before_expansion + 2:
            raise RuntimeError(
                'First item of the top-down view doesn\'t have at least two children')
        second_row_tree_item = tree_view_table.get_item_at(1, 0)
        third_row_tree_item = tree_view_table.get_item_at(2, 0)
        if (not ((second_row_tree_item.window_text().endswith('clone') and
                  third_row_tree_item.window_text() == '_start') or
                 (second_row_tree_item.window_text() == '_start') and
                 third_row_tree_item.window_text().endswith('clone'))):
            raise RuntimeError('Children of the first item of the top-down view '
                               'are not "*clone" and "_start"')
        logging.info('Verified that children of the first item are "*clone" and "_start"')


class VerifyTopDownContentForLoadedCapture(E2ETestCase):

    def _verify_row_count(self, tree_view_table: Table, expected_rows: int):
        self.expect_eq(tree_view_table.get_row_count(), expected_rows,
                       "Top-down view has {} rows".format(expected_rows))

    def _verify_first_rows(self, tree_view_table: Table, expectations: list[list[str]]):
        for i in range(len(expectations)):
            for j in range(len(expectations[i])):
                expectation = expectations[i][j]
                self.expect_eq(
                    tree_view_table.get_item_at(i, j).window_text(), expectation,
                    "Top-down view's cell ({}, {}) is '{}'".format(i, j, expectation))

    def _execute(self, selection_tab: bool = False):
        if not selection_tab:
            self.find_control('TabItem', 'Top-Down').click_input()
            logging.info("Switched to 'Top-Down' tab")
            tab = self.find_control('Group', 'topDownTab')
        else:
            self.find_control('TabItem', 'Top-Down (selection)').click_input()
            logging.info("Switched to 'Top-Down (selection)' tab")
            tab = self.find_control('Group', 'selectionTopDownTab')

        tree_view_table = Table(find_control(tab, 'Tree'))

        self.expect_eq(tree_view_table.get_column_count(), 6, "Top-down view has 6 columns")

        expected_thread_count = 20
        self._verify_row_count(tree_view_table, expected_thread_count)

        expectations = [
            ["OrbitTest (all threads)", "100.00% (1583)", "", "", "", ""],
            ["OrbitThread_203 [20301]", "79.34% (1256)", "", "", "", ""],
        ]
        self._verify_first_rows(tree_view_table, expectations)
        logging.info("Verified content of top-down view when all threads are collapsed")

        logging.info("Expanding a thread of the top-down view")
        tree_view_table.get_item_at(1, 0).double_click_input()
        expected_first_thread_child_count = 2
        self._verify_row_count(tree_view_table,
                               expected_thread_count + expected_first_thread_child_count)

        expectations = [
            ["OrbitTest (all threads)", "100.00% (1583)", "", "", "", ""],
            ["OrbitThread_203 [20301]", "79.34% (1256)", "", "", "", ""],
            ["clone", "79.22% (1254)", "0.00% (0)", "99.84%", "libc-2.24.so", "0x7fe86eff5900"],
            ["[unwind errors]", "0.13% (2)", "", "0.16%", "", ""],
        ]
        self._verify_first_rows(tree_view_table, expectations)
        logging.info("Verified content of top-down view when one thread is expanded")

        logging.info("Recursively expanding a thread of the top-down view")
        tree_view_table.get_item_at(1, 0).click_input(button='right')
        self.find_context_menu_item('Expand recursively').click_input()
        expected_first_thread_descendant_count = 19
        self._verify_row_count(tree_view_table,
                               expected_thread_count + expected_first_thread_descendant_count)

        expectations = [
            ["OrbitTest (all threads)", "100.00% (1583)", "", "", "", ""],
            ["OrbitThread_203 [20301]", "79.34% (1256)", "", "", "", ""],
            ["clone", "79.22% (1254)", "0.00% (0)", "99.84%", "libc-2.24.so", "0x7fe86eff5900"],
            [
                "start_thread", "79.22% (1254)", "0.00% (0)", "100.00%", "libpthread-2.24.so",
                "0x7fe86f9d0480"
            ],
            [
                "void* std::__1::__thread_proxy<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct> >, void (OrbitTestImpl::*)(), OrbitTestImpl*> >(void*)",
                "79.22% (1254)", "0.00% (0)", "100.00%", "OrbitTest", "0x55a44717c430"
            ],
        ]
        self._verify_first_rows(tree_view_table, expectations)
        logging.info("Verified content of top-down view when one thread is recursively expanded")

        logging.info("Expanding the entire top-down view")
        tree_view_table.get_item_at(0, 0).click_input(button='right')
        self.find_context_menu_item('Expand all').click_input()
        self._verify_row_count(tree_view_table, 827)

        expectations = [
            ["OrbitTest (all threads)", "100.00% (1583)", "", "", "", ""],
            ["clone", "97.79% (1548)", "0.00% (0)", "97.79%", "libc-2.24.so", "0x7fe86eff5900"],
            [
                "start_thread", "97.79% (1548)", "0.00% (0)", "100.00%", "libpthread-2.24.so",
                "0x7fe86f9d0480"
            ],
            [
                "void* std::__1::__thread_proxy<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct> >, void (OrbitTestImpl::*)(), OrbitTestImpl*> >(void*)",
                "79.72% (1262)", "0.00% (0)", "81.52%", "OrbitTest", "0x55a44717c430"
            ],
            [
                "OrbitTestImpl::Loop()", "79.22% (1254)", "0.00% (0)", "99.37%", "OrbitTest",
                "0x55a44717a950"
            ],
        ]
        self._verify_first_rows(tree_view_table, expectations)
        logging.info("Verified content of top-down view when it is completely expanded")

        search_term = 'fputs'
        logging.info("Searching top-down view for '{}'".format(search_term))
        search_bar = find_control(parent=tab, control_type='Edit', name='filter')
        search_bar.set_focus()
        send_keys(search_term)
        time.sleep(1)  # The top-down view waits for the user to stop typing before searching.
        self._verify_row_count(tree_view_table, 42)

        search_item_count = 0
        for i in range(tree_view_table.get_row_count()):
            if tree_view_table.get_item_at(i, 0).window_text() == search_term:
                search_item_count += 1
                expectations = [
                    "fputs", "0.06% (1)", "0.06% (1)", "100.00%", "libc-2.24.so", "0x7fe86ef70f90"
                ]
                for j in range(len(expectations)):
                    expectation = expectations[j]
                    self.expect_eq(
                        tree_view_table.get_item_at(i, j).window_text(), expectation,
                        "Top-down view's cell ({}, {}) is '{}'".format(i, j, expectation))
        self.expect_eq(search_item_count, 2,
                       "Searching top-down view for '{}' produces two results".format(search_term))
        logging.info("Verified result of searching top-down view for '{}'".format(search_term))
