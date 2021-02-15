"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging
from typing import Tuple

from pywinauto.base_wrapper import BaseWrapper

from core.orbit_e2e import E2ETestCase, wait_for_condition

from test_cases.capture_window import MatchTracks, CheckTimers


class LiveTabTestCase(E2ETestCase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._live_tab = None

    def execute(self, suite):
        self._live_tab = self.find_control("Group", "liveTab", parent=suite.top_window())
        super().execute(suite)

    def find_function_cell(self, function_name) -> Tuple[BaseWrapper, int]:
        children = self.find_control('Tree', parent=self._live_tab).children()
        for i in range(len(children)):
            if function_name in children[i].window_text():
                return children[i], i
        return None, -1


class AddIterator(LiveTabTestCase):
    """
    Add a single iterator to an already hooked function and verify its existence.
    """
    def _execute(self, function_name):
        # Adding an iterator adds three new buttons and we use this knowledge to
        # verify that an iterator was added correctly. It seems other widgets like
        # the label are not well represented by pywinauto.
        BUTTONS_PER_ITERATOR = 3
        count_all_buttons_before = len(self._live_tab.descendants(control_type='Button'))

        # Find function_name in the live tab and add an iterator
        cell, _ = self.find_function_cell(function_name)
        self.expect_true(cell is not None, 'Found row containing function "%s"' % function_name)
        cell.click_input(button='right')
        self.find_context_menu_item('Add iterator(s)').click_input()

        count_all_buttons_after = len(self._live_tab.descendants(control_type='Button'))
        if count_all_buttons_before + BUTTONS_PER_ITERATOR != count_all_buttons_after:
            raise RuntimeError('Iterator not correctly added')


class AddFrameTrack(LiveTabTestCase):
    """
    Add a frame track to an already hooked function and verify its existence
    """
    def _execute(self, function_name):
        cell, index = self.find_function_cell(function_name)
        cell.click_input(button='right')
        self.find_context_menu_item('Enable frame track(s)').click_input()

        tree_view = self.find_control('Tree', parent=self._live_tab)
        wait_for_condition(lambda: "F" in tree_view.children()[index - 1].window_text())

        logging.info("Verifying existence of track in the Capture window")
        match_tracks = MatchTracks(expected_names=["Frame track based on %s" % function_name],
                                   allow_additional_tracks=True)
        match_tracks.execute(self.suite)

        check_timers = CheckTimers(track_name_contains=function_name)
        check_timers.execute(self.suite)


class VerifyFunctionCallCount(LiveTabTestCase):
    """
    Verify the amount of times a hooked function has been called according to the
    live-tab
    """
    def _execute(self, function_name, min_calls=1, max_calls=pow(2, 33)-1):
        cell, index = self.find_function_cell(function_name)
        children = self.find_control('Tree', parent=self._live_tab).children()

        call_count = int(children[index + 1].window_text())
        hook_status = children[index - 1].window_text()

        logging.info('Found a call count of %s, hook status: %s', call_count, hook_status)
        self.expect_true(min_calls <= call_count <= max_calls,
                         'Call count for function "%s" expected to be between %s and %s, was %s'
                         % (function_name, min_calls, max_calls, call_count))
                         
class VerifyOneFunctionWasCalled(LiveTabTestCase):
    """
    Verify that at least one of the functions matching the function name has 
    received the given number of hits.
    """
    def _execute(self, function_name_contains, min_calls=1, max_calls=pow(2, 33)-1):
        children = self.find_control('Tree', parent=self._live_tab).children()
        for i in range(len(children)):
            if function_name_contains in children[i].window_text():
                call_count = int(children[i + 1].window_text())
                if min_calls <= call_count <= max_calls:
                    logging.info('Found a call to "%s" with %s hits', 
                                 children[i].window_text(), call_count)
                    return
        
        raise RuntimeError('No function matching "%s" has received the required hit count' % (function_name))
