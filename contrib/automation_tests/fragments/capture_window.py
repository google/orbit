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
    def __init__(self, track_index=-1):
        self._track_index = track_index
        self._time_graph = None

    def _execute(self):
        self._time_graph = find_time_graph(self.e2e_test.top_window())
        track = get_track_components(self._time_graph.children()[self._track_index])

        self.expect_true(not track.Container.has_keyboard_focus(), "Track is not selected")
        track.Title.click_input()
        self.expect_true(track.Container.has_keyboard_focus(), "Track is selected")


class MoveTrack(Fragment):
    def __init__(self, track_index=-1, new_y=0, expected_new_index=0):
        self._track_index = track_index
        self._new_y = new_y
        self._expected_new_index = expected_new_index
        self._time_graph = None

    def _execute(self):
        self._time_graph = find_time_graph(self.e2e_test.top_window())

        track = get_track_components(self._time_graph.children()[self._track_index])
        mouse_x = track.Title.client_to_screen((5, 0))[0]
        logging.info("Moving track '%s' from index %s to y=%s (expected new index: %s)",
                     track.Container.texts()[0], self._track_index, self._new_y, self._expected_new_index)
        track.Title.drag_mouse_input((mouse_x, self._new_y + 5))

        index = self._time_graph.children().index(track.Container)
        self.expect_eq(index, self._expected_new_index, "Expected track index after reordering")
