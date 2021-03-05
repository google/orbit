"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging

from core.orbit_e2e import E2ETestCase, find_control


class VerifyHelloGgpTopDownContents(E2ETestCase):
    """
    TODO: This could be updated to check the contents more thoroughly and dynamically
    react to the amount of columns in the treeview - see common_controls.DataViewPanel
    on how to access the required fields.
    """
    def _execute(self):
        self.find_control("TabItem", "Top-Down").click_input()
        logging.info('Switched to Top-Down tab')

        tree_view = find_control(self.find_control('Group', 'topDownTab'), 'Tree')

        # tree_view.children(control_type='TreeItem') returns
        # every cell in the top-down view, in order by row and then column.
        # It can take a few seconds.
        logging.info('Listing items of the top-down view...')
        tree_items = tree_view.children(control_type='TreeItem')
        TOP_DOWN_ROW_CELL_COUNT = 6
        row_count_before_expansion = len(tree_items) / TOP_DOWN_ROW_CELL_COUNT

        if row_count_before_expansion < 3:
            raise RuntimeError('Less than 3 rows in the top-down view')

        if (not tree_items[0].window_text().startswith('hello_') or
                not tree_items[0].window_text().endswith(' (all threads)')):
            raise RuntimeError(
                'First item of the top-down view is not "hello_* (all threads)"')
        logging.info('Verified that first item is "hello_* (all threads)"')

        if ((not tree_items[TOP_DOWN_ROW_CELL_COUNT].window_text().startswith(
                'hello_') and
             not tree_items[TOP_DOWN_ROW_CELL_COUNT].window_text().startswith('Ggp')
        ) or not tree_items[TOP_DOWN_ROW_CELL_COUNT].window_text().endswith(']')
        ):
            raise RuntimeError(
                'Second item of the top-down view is not "hello_*]" nor "Ggp*]"')
        logging.info('Verified that second item is "hello_*]" or "Ggp*]"')

        tree_items[0].double_click_input()
        logging.info('Expanded the first item')

        logging.info('Re-listing items of the top-down view...')
        tree_items = tree_view.children(control_type='TreeItem')
        row_count_after_expansion = len(tree_items) / TOP_DOWN_ROW_CELL_COUNT

        if row_count_after_expansion < row_count_before_expansion + 2:
            raise RuntimeError(
                'First item of the top-down view doesn\'t have at least two children'
            )
        if (not (
                (tree_items[TOP_DOWN_ROW_CELL_COUNT].window_text().endswith('clone') and
                 tree_items[2 * TOP_DOWN_ROW_CELL_COUNT].window_text() == '_start') or
                (tree_items[TOP_DOWN_ROW_CELL_COUNT].window_text() == '_start') and
                tree_items[2 * TOP_DOWN_ROW_CELL_COUNT].window_text().endswith(
                    'clone'))):
            raise RuntimeError('Children of the first item of the top-down view '
                               'are not "*clone" and "_start"')
        logging.info(
            'Verified that children of the first item are "*clone" and "_start"')