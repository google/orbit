"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging
from typing import Tuple

from pywinauto.base_wrapper import BaseWrapper

from core.orbit_e2e import E2ETestCase, wait_for_condition


class LiveTabTestCase(E2ETestCase):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._live_tab = None

    def execute(self, suite):
        self.find_control("TabItem", "Live", parent=suite.top_window()).click_input()
        self._live_tab = self.find_control("Group", "liveTab", parent=suite.top_window())
        super().execute(suite)

    def _find_scope_cell(self, scope_name) -> Tuple[BaseWrapper, int]:
        children = self.find_control('Tree', parent=self._live_tab).children()
        for i in range(len(children)):
            if scope_name in children[i].window_text():
                return children[i], i
        return None, -1


class AddIterator(LiveTabTestCase):
    """
    Add a single iterator to an already hooked function and verify its existence.
    """

    def _execute(self, function_name):
        # Bring iterator tab to foreground.
        self.find_control("TabItem", "Iterators", parent=self._live_tab).click_input()

        # Adding an iterator adds three new buttons and we use this knowledge to
        # verify that an iterator was added correctly. It seems other widgets like
        # the label are not well represented by pywinauto.
        BUTTONS_PER_ITERATOR = 3
        count_all_buttons_before = len(self._live_tab.descendants(control_type='Button'))

        # Find function_name in the live tab and add an iterator
        cell, _ = self._find_scope_cell(function_name)
        self.expect_true(cell is not None, 'Found row containing function "%s"' % function_name)
        cell.click_input(button='right')
        self.find_context_menu_item('Add iterator(s)').click_input()

        count_all_buttons_after = len(self._live_tab.descendants(control_type='Button'))
        if count_all_buttons_before + BUTTONS_PER_ITERATOR != count_all_buttons_after:
            raise RuntimeError('Iterator not correctly added')


class AddFrameTrack(LiveTabTestCase):
    """
    Add a frame track to an already hooked function
    """

    def _execute(self, function_name):
        cell, index = self._find_scope_cell(function_name)
        cell.click_input(button='right')
        self.find_context_menu_item('Enable frame track(s)').click_input()

        tree_view = self.find_control('Tree', parent=self._live_tab)
        wait_for_condition(lambda: "F" in tree_view.children()[index - 1].window_text())


class VerifyScopeTypeAndHitCount(LiveTabTestCase):
    """
    Verify the amount of times a scope has been hit and its type 
    according to the live-tab.
    "Scope" is any entity that we display aggregated statistics for
    under the Live tab. It could be Dynamic instrumentation (type "D")
    or Manual (types "MS" and "MA" for synchronous and asynchronous respectively).
    """

    def _execute(self, scope_name, scope_type, min_hits=1, max_hits=pow(2, 33) - 1):
        cell, index = self._find_scope_cell(scope_name)
        children = self.find_control('Tree', parent=self._live_tab).children()

        hit_count = int(children[index + 1].window_text())
        actual_scope_type = children[index - 1].window_text()

        logging.info('Found a hit count of %s, scope type: %s', hit_count, actual_scope_type)
        self.expect_true(
            min_hits <= hit_count <= max_hits,
            'Hit count for scope "%s" expected to be between %s and %s, was %s' %
            (scope_name, min_hits, max_hits, hit_count))
        self.expect_true(
            actual_scope_type.startswith(scope_type),
            'Scope "%s" expected to be of type %s , was %s' % (scope_name, scope_type, scope_type))


class VerifyOneFunctionWasHit(LiveTabTestCase):
    """
    Verify that at least one of the functions matching the function name has 
    received the given number of hits.
    """

    def _execute(self, function_name_contains, min_calls=1, max_calls=pow(2, 33) - 1):
        children = self.find_control('Tree', parent=self._live_tab).children()
        for i in range(len(children)):
            if function_name_contains in children[i].window_text():
                call_count = int(children[i + 1].window_text())
                if min_calls <= call_count <= max_calls:
                    logging.info('Found a call to "%s" with %s hits', children[i].window_text(),
                                 call_count)
                    return

        raise RuntimeError('No function matching "%s" has received the required hit count' %
                           (function_name_contains))
