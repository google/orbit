"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""
import logging

from core.common_controls import Table
from core.orbit_e2e import E2ETestCase, find_control


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
