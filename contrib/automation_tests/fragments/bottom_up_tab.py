"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""
import logging

from core.orbit_e2e import E2ETestCase, find_control


class VerifyHelloGgpBottomUpContents(E2ETestCase):
    """
    TODO: This could be updated to check the contents more thoroughly and dynamically
    react to the amount of columns in the treeview - see common_controls.DataViewPanel
    on how to access the required fields.
    """
    def _execute(self):
        self.find_control("TabItem", "Bottom-Up").click_input()
        logging.info('Switched to Bottom-Up tab')

        # main_wnd.TreeView.children(control_type='TreeItem') returns
        # every cell in the bottom-up view, in order by row and then column.
        # It can take a few seconds.
        logging.info('Listing items of the bottom-up view...')
        tree_view = find_control(self.find_control('Group', 'bottomUpTab'), 'Tree')
        tree_items = tree_view.children(control_type='TreeItem')
        BOTTOM_UP_ROW_CELL_COUNT = 5
        row_count = len(tree_items) / BOTTOM_UP_ROW_CELL_COUNT

        if row_count < 10:
            raise RuntimeError('Less than 10 rows in the bottom-up view')

        if tree_items[0].window_text() != 'ioctl':
            raise RuntimeError('First item of the bottom-up view is not "ioctl"')
        logging.info('Verified that first item is "ioctl"')

        tree_items[0].double_click_input()
        logging.info('Expanded the first item')

        logging.info('Re-listing items of the bottom-up view...')
        tree_items = tree_view.children(control_type='TreeItem')

        if not tree_items[BOTTOM_UP_ROW_CELL_COUNT].window_text().startswith('drm'):
            raise RuntimeError('First child of the first item ("ioctl") '
                               'of the bottom-up view doesn\'t start with "drm"')
        logging.info(
            'Verified that first child of the first item starts with "drm"')