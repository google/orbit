"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""
import logging

from core.common_controls import Table
from core.orbit_e2e import E2ETestCase, find_control


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
