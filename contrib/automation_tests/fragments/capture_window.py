"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging

from core.orbit_e2e import Fragment, find_control
from collections import namedtuple
from pywinauto.base_wrapper import BaseWrapper


def find_time_graph(hwnd) -> BaseWrapper:
    return find_control(hwnd, 'Image', name='TimeGraph')


Track = namedtuple('Track', ['Container', 'Title', 'Content'])


def get_track_components(track_control) -> Track:
    return Track(Container=track_control,
                 Title=find_control(track_control, 'TabItem'),
                 Content=find_control(track_control, 'Group'))


class SelectTrack(Fragment):
    def __init__(self, track_index=-1, check_selection_before=True, expect_failure=False):
        super().__init__(track_index=track_index, check_selection_before=check_selection_before,
                         expect_failure=expect_failure)

    def _execute(self, track_index, check_selection_before, expect_failure):
        time_graph = find_time_graph(self.e2e_test.top_window())
        track = get_track_components(time_graph.children()[track_index])

        if check_selection_before:
            self.expect_true(not track.Container.has_keyboard_focus(), "Track is not selected")
        track.Title.click_input()
        if expect_failure:
            self.expect_true(not track.Container.has_keyboard_focus(), "Track is not selected")
        else:
            self.expect_true(track.Container.has_keyboard_focus(), "Track is selected")


class MoveTrack(Fragment):
    def __init__(self, track_index=-1, new_y=0, expected_new_index=0):
        super().__init__(track_index=track_index, new_y=new_y, expected_new_index=expected_new_index)

    def _execute(self, track_index, new_y, expected_new_index):
        time_graph = find_time_graph(self.e2e_test.top_window())

        track = get_track_components(time_graph.children()[track_index])
        mouse_x = track.Title.client_to_screen((5, 0))[0]
        logging.info("Moving track '%s' from index %s to y=%s (expected new index: %s)",
                     track.Container.texts()[0], track_index, new_y, expected_new_index)
        track.Title.drag_mouse_input((mouse_x, new_y + 5))

        index = time_graph.children().index(track.Container)
        self.expect_eq(index, expected_new_index, "Expected track index after reordering")
