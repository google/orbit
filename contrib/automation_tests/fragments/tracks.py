"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging

from typing import Tuple, List

from core.orbit_e2e import Fragment, find_control, E2ETest
from collections import namedtuple
from pywinauto.base_wrapper import BaseWrapper
from pywinauto import mouse


Track = namedtuple('Track', ['Container', 'Title', 'Content'])


def get_track_components(track_control) -> Track:
    """
    Utility method to split a Track into the main container, the title tab, and the content.
    """
    return Track(Container=track_control,
                 Title=find_control(track_control, 'TabItem'),
                 Content=find_control(track_control, 'Group'))


class CaptureWindowFragment(Fragment):
    """
    Base class for fragments interacting with the capture window, provides common functionality.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._time_graph = None

    def execute(self, e2e_test: E2ETest):
        self._time_graph = find_control(e2e_test.top_window(), 'Image', name='TimeGraph')
        super().execute(e2e_test=e2e_test)

    def _find_tracks(self):
        return self._time_graph.children()


class SelectTrack(CaptureWindowFragment):
    """
    Select a track in the capture window and check the selection afterwards.
    """

    def _execute(self, track_index: int = 0, check_selection_before: bool = False, expect_failure: bool = False):
        """
        :param track_index: Index of the track to select
        :param check_selection_before: If True, verifies that this track was not selected before
        :param expect_failure: If True, it is expected that the track in question is NOT selected after executing this
        """
        track = get_track_components(self._find_tracks()[track_index])

        if check_selection_before:
            self.expect_true(not track.Container.has_keyboard_focus(), "Track is not selected")
        logging.info("Selecting track with index %s", track_index)
        track.Title.click_input()
        if expect_failure:
            self.expect_true(not track.Container.has_keyboard_focus(), "Track is not selected")
        else:
            self.expect_true(track.Container.has_keyboard_focus(), "Track is selected")


class DeselectTrack(CaptureWindowFragment):
    """
    Deselect the currently selected track in the capture window, verify no track is selected afterwards.
    Expects a track to be selected, fails otherwise.
    """

    def _find_focused_track_and_index(self) -> Tuple[BaseWrapper, int]:
        children = self._find_tracks()
        for i in range(len(children)):
            if children[i].has_keyboard_focus():
                return children[i], i
        return None, None

    def _execute(self):
        track, index = self._find_focused_track_and_index()

        self.expect_true(index is not None, "A track is selected")
        # Click slightly above the track to hit any empty space
        rect = track.rectangle()
        mouse.click(button='left', coords=(rect.left, rect.top - 5))
        # TODO: I need to call this twice to work, why?
        mouse.click(button='left', coords=(rect.left, rect.top - 5))
        self.expect_true(not self._find_tracks()[index].has_keyboard_focus(), "Track is no longer selected")
        self.expect_true(self._find_focused_track_and_index()[0] is None, "No Track is selected")


class MoveTrack(CaptureWindowFragment):
    """
    Click and drag a track in the capture window, verify ordering of tracks afterwards.
    """

    def _execute(self, track_index: int = 0, new_index: int = 0, expected_new_index: int = None):
        """
        :param track_index: Track index (before dragging) to move
        :param new_index: New index the track should be moved to
        :param expected_new_index: Optional - Expected index of the track after moving. If None, it is expected that
            the track will be at position <new_index> after execution.
        """
        if expected_new_index is None:
            expected_new_index = new_index

        # Drag 1 px above the target track index if moving up, or 1 px below if moving down
        tracks = self._find_tracks()
        rect = tracks[new_index].rectangle()
        track_count = len(tracks)
        if track_index % track_count >= new_index % track_count:
            new_y = rect.top - 1
        else:
            new_y = rect.bottom + 1

        track = get_track_components(tracks[track_index])
        mouse_x = track.Title.rectangle().left + 5
        logging.info("Moving track '%s' from index %s to y=%s (expected new index: %s)",
                     track.Container.texts()[0], track_index, new_y, expected_new_index)

        track.Title.drag_mouse_input((mouse_x, new_y + 5))

        index = self._find_tracks().index(track.Container)
        self.expect_eq(index, expected_new_index % track_count, "Expected track index after reordering")


class MatchTracks(CaptureWindowFragment):
    """
    Verify that the existing visible tracks match the expected tracks
    """
    def _execute(self, expected_count: int = None, expected_names: List[str] = None):
        """
        You need to pass either expected_track_count, or expected_name_list. If both are passed, they need to match
        w.r.t. expected number of tracks.
        :param expected_count: # of tracks to be visible
        :param expected_names: List of (partial) matching names of tracks to be visible.
            The name is a partial match, but the exact number of tracks is expected.
        """
        assert (expected_count is not None or expected_names)
        if expected_count is not None and expected_names:
            assert (expected_count == len(expected_names))
        if expected_count is None:
            expected_count = len(expected_names)

        tracks = self._find_tracks()

        self.expect_eq(len(tracks), expected_count, "# of tracks matches %s" % expected_count)

        if expected_names:
            logging.info("Matching visible tracks against name list (%s)", expected_names)
            for name in expected_names:
                found = False
                for track in tracks:
                    track_name = track.texts()[0]
                    if name in track_name:
                        found = True
                        break
                self.expect_true(found, "Found a match for track name '%s'" % name)


class FilterTracks(CaptureWindowFragment):
    """
    Set a filter in the capture tab, and verify either the amount of visible tracks or their names
    """

    def _execute(self, filter_string: str = "", expected_count: int = None, expected_names: List[str] = None):
        """
        See MatchTracks._execute() for documentation.
        """
        toolbar = find_control(self.e2e_test.top_window(), "ToolBar", name="CaptureToolBar")
        track_filter = find_control(toolbar, "Edit", "FilterTracks")

        logging.info("Setting track filter text: '%s'", filter_string)
        track_filter.set_edit_text(filter_string)
        tracks = self._find_tracks()

        # Verify by re-using a MatchTracks Fragment
        match_tracks = MatchTracks(expected_count=expected_count, expected_names=expected_names)
        match_tracks.execute(self.e2e_test)
