"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""
import logging
import time

from core.common_controls import Table
from core.orbit_e2e import E2ETestCase, find_control, OrbitE2EError
from pywinauto.keyboard import send_keys
from typing import Sequence


class SelectAllCallstackFromTopDownTab(E2ETestCase):
    """
    Select all CallstackEvents from the Top-Down tab by using the "Select these callstacks" context menu action.
    """

    def _execute(self):
        self.find_control("TabItem", "Top-Down").click_input()
        logging.info("Switched to Top-Down tab")
        tree_view_table = Table(find_control(self.find_control('Group', 'topDownTab'), 'Tree'))
        tree_view_table.get_item_at(0, 0).click_input()
        # Select all threads. Selecting the "(all threads)" node would be enough, but this way at a later state we can
        # also verify that the selection doesn't create duplicates.
        send_keys("^a")
        tree_view_table.get_item_at(0, 0).click_input('right')
        self.find_context_menu_item("Select these callstacks").click_input()


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
    """
    Verify the content of the top-down view for the capture loaded from "OrbitTest_1-72.orbit".
    In particular, we verify the number of rows and the content of the first rows when the tree is expanded in a few
    different ways. Then we search for a function and verify that the tree is expanded to show it.
    """

    def _verify_row_count(self, tree_view_table: Table, expected_rows: int):
        self.expect_eq(tree_view_table.get_row_count(), expected_rows,
                       "Top-down view has {} rows".format(expected_rows))

    def _verify_first_rows(self, tree_view_table: Table, expectations: Sequence[Sequence[str]]):
        for i in range(len(expectations)):
            for j in range(len(expectations[i])):
                expectation = expectations[i][j]
                self.expect_eq(
                    tree_view_table.get_item_at(i, j).window_text(), expectation,
                    "Top-down view's cell ({}, {}) is '{}'".format(i, j, expectation))

    def _switch_to_tab(self, selection_tab: bool = False):
        if not selection_tab:
            self.find_control('TabItem', 'Top-Down').click_input()
            logging.info("Switched to 'Top-Down' tab")
            return self.find_control('Group', 'topDownTab')
        else:
            self.find_control('TabItem', 'Top-Down (selection)').click_input()
            logging.info("Switched to 'Top-Down (selection)' tab")
            return self.find_control('Group', 'selectionTopDownTab')

    def _collapse_tree(self, tree_view_table: Table):
        tree_view_table.get_item_at(0, 0).click_input(button='right')
        self.find_context_menu_item("Collapse all").click_input()

    def _verify_columns(self, tree_view_table):
        self.expect_eq(tree_view_table.get_column_count(), 6, "Top-down view has 6 columns")

    EXPECTED_THREAD_COUNT = 20

    def _verify_rows_when_tree_collapsed(self, tree_view_table):
        self._verify_row_count(tree_view_table, self.EXPECTED_THREAD_COUNT)

        expectations = [
            ["OrbitTest (all threads)", "100.00% (1583)", "", "", "", ""],
            ["OrbitThread_203 [20301]", "79.34% (1256)", "", "", "", ""],
        ]
        self._verify_first_rows(tree_view_table, expectations)
        logging.info("Verified content of top-down view when all threads are collapsed")

    def _verify_rows_when_thread_and_errors_expanded(self, tree_view_table):
        logging.info("Expanding a thread of the top-down view")
        tree_view_table.get_item_at(1, 0).double_click_input()
        expected_first_thread_child_count = 2
        expected_total_child_count = self.EXPECTED_THREAD_COUNT + expected_first_thread_child_count
        self._verify_row_count(tree_view_table, expected_total_child_count)

        logging.info("Expanding an [Unwind errors] node of the top-down view")
        tree_view_table.get_item_at(3, 0).double_click_input()
        expected_unwind_errors_child_count = 1
        expected_total_child_count += expected_unwind_errors_child_count
        self._verify_row_count(tree_view_table, expected_total_child_count)

        logging.info("Expanding an error type node of the top-down view")
        tree_view_table.get_item_at(4, 0).double_click_input()
        expected_unwind_error_type_child_count = 1
        expected_total_child_count += expected_unwind_error_type_child_count
        self._verify_row_count(tree_view_table, expected_total_child_count)

        expectations = [
            ["OrbitTest (all threads)", "100.00% (1583)", "", "", "", ""],
            ["OrbitThread_203 [20301]", "79.34% (1256)", "", "", "", ""],
            ["clone", "79.22% (1254)", "0.00% (0)", "99.84%", "libc-2.24.so", "0x7fe86eff5900"],
            ["[Unwind errors]", "0.13% (2)", "", "0.16%", "", ""],
            ["Callstack inside user-space instrumentation", "0.13% (2)", "", "100.00%", "", ""],
            [
                "decltype(auto) std::__1::__variant_detail::__visitation::__base::__dispatcher<1ul>::__dispatch<std"
                "::__1::__variant_detail::__destructor<std::__1::__variant_detail::__traits<orbit_grpc_protos"
                "::FunctionEntry, orbit_grpc_protos::FunctionExit>, "
                "(std::__1::__variant_detail::_Trait)1>::__destroy()::'lambda'(auto&)&&, "
                "std::__1::__variant_detail::__base<(std::__1::__variant_detail::_Trait)1, "
                "orbit_grpc_protos::FunctionEntry, orbit_grpc_protos::FunctionExit>&>(auto, "
                "(std::__1::__variant_detail::_Trait)1...)", "0.13% (2)", "0.13% (2)", "100.00%",
                "liborbituserspaceinstrumentation.so", "0x7fe84851ab10"
            ]
        ]
        self._verify_first_rows(tree_view_table, expectations)
        logging.info("Verified content of top-down view when one thread is expanded")

    def _verify_rows_when_thread_recursively_expanded(self, tree_view_table):
        logging.info("Recursively expanding a thread of the top-down view")
        tree_view_table.get_item_at(1, 0).click_input(button='right')
        self.find_context_menu_item('Expand recursively\tALT+Click').click_input()
        expected_first_thread_descendant_count = 20
        self._verify_row_count(tree_view_table,
                               self.EXPECTED_THREAD_COUNT + expected_first_thread_descendant_count)

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

    def _verify_rows_when_tree_expanded(self, tree_view_table):
        logging.info("Expanding the entire top-down view")
        tree_view_table.get_item_at(0, 0).click_input(button='right')
        self.find_context_menu_item('Expand all').click_input()
        self._verify_row_count(tree_view_table, 832)

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

    def _verify_rows_on_search(self, tab, tree_view_table):
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

        # Clear the search terms.
        search_bar.set_focus()
        send_keys('^a{BACKSPACE}')

    def _execute(self, selection_tab: bool = False):
        tab = self._switch_to_tab(selection_tab)
        tree_view_table = Table(find_control(tab, 'Tree'))

        self._verify_columns(tree_view_table)
        self._verify_rows_when_tree_collapsed(tree_view_table)
        self._verify_rows_when_thread_and_errors_expanded(tree_view_table)
        self._collapse_tree(tree_view_table)
        self._verify_rows_when_thread_recursively_expanded(tree_view_table)
        self._collapse_tree(tree_view_table)
        self._verify_rows_when_tree_expanded(tree_view_table)
        self._collapse_tree(tree_view_table)
        self._verify_rows_on_search(tab, tree_view_table)


class VerifyTopDownContentForTriangleExe(E2ETestCase):
    """
    Verify that the two outermost functions are `RtlUserThreadStart` (Windows) and `*clone` (Linux) from the
    "(all threads)" node of the top-down view.
    Verify the start function and some of its callees for the "triangle.exe" and "dxvk-submit" threads.
    """

    def _execute(self):
        self.find_control("TabItem", "Top-Down").click_input()
        logging.info("Switched to the 'Top-Down' tab")
        tab = self.find_control('Group', 'topDownTab')
        tree_view_table = Table(self.find_control('Tree', parent=tab))

        self._verify_all_threads(tree_view_table)
        self._collapse_tree(tree_view_table)
        self._verify_triangle_exe_thread(tree_view_table)
        self._collapse_tree(tree_view_table)
        self._verify_dxvk_submit_thread(tree_view_table)

    def _collapse_tree(self, tree_view_table: Table):
        tree_view_table.get_item_at(0, 0).click_input(button='right')
        self.find_context_menu_item("Collapse all").click_input()

    def _verify_all_threads(self, tree_view_table: Table):
        first_row_tree_item = tree_view_table.get_item_at(0, 0)
        self.expect_eq(first_row_tree_item.window_text(), "wine64-preloader (all threads)",
                       "First item of the top-down view is 'wine64-preloader (all threads)'")

        logging.info("Expanding the '(all threads)' node of the top-down view")
        row_count_before_expansion = tree_view_table.get_row_count()
        first_row_tree_item.double_click_input()
        row_count_after_expansion = tree_view_table.get_row_count()
        self.expect_true(row_count_after_expansion - row_count_before_expansion >= 2,
                         "The '(all threads)' node has at least two children")

        self.expect_eq(
            tree_view_table.get_item_at(1, 0).window_text(), "RtlUserThreadStart",
            "The most common outermost function is 'RtlUserThreadStart'")
        self.expect_true(
            tree_view_table.get_item_at(2, 0).window_text().endswith("clone"),
            "The second most common outermost function is '*clone'")
        logging.info("Verified the outermost functions from the top-down view")

    def _verify_triangle_exe_thread(self, tree_view_table: Table):
        thread_name = "triangle.exe"
        logging.info(f"Verifying the top outer functions for the thread '{thread_name}'")
        thread_row = self._find_thread_row(tree_view_table, thread_name)
        self._verify_top_outer_functions(tree_view_table, thread_row, [
            "RtlUserThreadStart", "BaseThreadInitThunk", "__scrt_common_main_seh", "wWinMain",
            "dxvk::DxgiSwapChain::Present1"
        ])
        logging.info(f"Verified the top outer functions for the thread '{thread_name}'")

    def _verify_dxvk_submit_thread(self, tree_view_table: Table):
        thread_name = "dxvk-submit"
        logging.info(f"Verifying the top outer functions for the thread '{thread_name}'")
        thread_row = self._find_thread_row(tree_view_table, thread_name)
        self._verify_top_outer_functions(tree_view_table, thread_row, [
            "RtlUserThreadStart", "BaseThreadInitThunk", "dxvk::ThreadFn::threadProc",
            "dxvk::DxvkSubmissionQueue::submitCmdLists", "dxvk::vk::Presenter::presentImage",
            "wine_vkAcquireNextImageKHR", "ggpdrv::(anonymous namespace)::AcquireNextImageKHR",
            "yeti::internal::vulkan::Swapchain::AcquireNextImage"
        ])
        logging.info(f"Verified the top outer functions for the thread '{thread_name}'")

    @staticmethod
    def _find_thread_row(tree_view_table: Table, thread_name: str):
        for row in range(tree_view_table.get_row_count()):
            if tree_view_table.get_item_at(row, 0).window_text().startswith(thread_name + " ["):
                return row
        raise OrbitE2EError(f"The top-down view has no '{thread_name}' thread")

    def _verify_top_outer_functions(self, tree_view_table: Table, thread_row: int,
                                    expected_functions: Sequence[str]):
        for index, expected in enumerate(expected_functions):
            # Expand the previous node (thread or caller of the expected function).
            parent_row = thread_row + index
            parent_item = tree_view_table.get_item_at(parent_row, 0)
            parent_text = parent_item.window_text()
            row_count_before_expansion = tree_view_table.get_row_count()
            parent_item.double_click_input()
            row_count_after_expansion = tree_view_table.get_row_count()
            self.expect_true(row_count_after_expansion > row_count_before_expansion,
                             f"The '{parent_text}' node has at least one child")

            # Verify the function name.
            row = parent_row + 1
            self.expect_true(
                tree_view_table.get_item_at(row, 0).window_text().startswith(expected),
                f"The top ${index + 1}-th outer function is '${expected}'")
