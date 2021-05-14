"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging
import time
from fnmatch import fnmatch

from typing import Tuple, List, Iterable

from core.common_controls import Track
from core.orbit_e2e import E2ETestCase, E2ETestSuite, wait_for_condition
from pywinauto import mouse, keyboard
from pywinauto.base_wrapper import BaseWrapper


class CaptureWindowE2ETestCaseBase(E2ETestCase):
    """
    Base class for fragments interacting with the capture window, provides common functionality.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._time_graph = None

    def execute(self, suite: E2ETestSuite):
        self._time_graph = self.find_control('Image', name='TimeGraph', parent=suite.top_window())
        super().execute(suite=suite)

    def _find_tracks(self, name_filter: str = None, recursive: bool = False):
        if not recursive:
            tracks = self._time_graph.children()
            if name_filter is not None:
                tracks = filter(lambda track: fnmatch(track.texts()[0], name_filter), tracks)
            return list(tracks)

        work_list = self._time_graph.children()
        tracks = list()

        while work_list:
            track = work_list.pop()
            if name_filter is None or fnmatch(track.texts()[0], name_filter):
                tracks.append(track)
            # Find all sub tracks of this track. We identify a track by its first child being a tab.
            for sub_track in track.children():
                if self.find_control("TabItem", parent=sub_track, raise_on_failure=False):
                    work_list.append(sub_track)

        return list(tracks)


class SelectTrack(CaptureWindowE2ETestCaseBase):
    """
    Select a track in the capture window and check the selection afterwards.
    """

    def _execute(self, track_index: int = 0, check_selection_before: bool = False, expect_failure: bool = False):
        """
        :param track_index: Index of the track to select
        :param check_selection_before: If True, verifies that this track was not selected before
        :param expect_failure: If True, it is expected that the track in question is NOT selected after executing this
        """
        track = Track(self._find_tracks()[track_index])

        if check_selection_before:
            self.expect_true(not track.container.has_keyboard_focus(), "Track is not selected")
        logging.info("Selecting track with index {}".format(track_index))
        track.title.click_input()
        if expect_failure:
            self.expect_true(not track.container.has_keyboard_focus(), "Track is not selected")
        else:
            self.expect_true(track.container.has_keyboard_focus(), "Track is selected")


class DeselectTrack(CaptureWindowE2ETestCaseBase):
    """
    Deselect the currently selected track in the capture window, verify no track is selected afterwards.
    Expects a track to be selected, fails otherwise.
    """

    def _find_focused_track_and_index(self) -> Tuple[BaseWrapper or None, int or None]:
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
        mouse.click(button='left', coords=(rect.left + 10, rect.top - 5))
        # TODO: I need to call this twice to work, why?
        mouse.click(button='left', coords=(rect.left + 10, rect.top - 5))
        self.expect_true(not self._find_tracks()[index].has_keyboard_focus(), "Track is no longer selected")
        self.expect_true(self._find_focused_track_and_index()[0] is None, "No Track is selected")


class MoveTrack(CaptureWindowE2ETestCaseBase):
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

        track = Track(tracks[track_index])
        mouse_x = track.title.rectangle().left + 5
        logging.info("Moving track '{}' from index {} to y={} (expected new index: {})".format(
            track.container.texts()[0], track_index, new_y, expected_new_index))

        track.title.drag_mouse_input((mouse_x, new_y + 5))

        index = self._find_tracks().index(track.container)
        self.expect_eq(index, expected_new_index % track_count,
                       "Expected track index {} after reordering, got {}".format(expected_new_index, index))


class MatchTracks(CaptureWindowE2ETestCaseBase):
    """
    Verify that the existing visible tracks match the expected tracks
    """

    def _execute(self, expected_count: int = None, expected_names: List[str or Iterable[str]] = None,
                 allow_additional_tracks=False):
        """
        You need to pass either expected_track_count, or expected_name_list. If both are passed, they need to match
        w.r.t. expected number of tracks.
        :param expected_count: # of tracks to be visible
        :param expected_names: List of (partial) matching names of tracks to be visible.
            The name is a partial match, but the exact number of tracks is expected. Each entry in this list can either
            be a string or a list of strings. If an entry is a list of strings, the test expects at least on of the
            names to be matched (i.e. this is an "or" condition on multiple possible track names)
        :param allow_additional_tracks: If True, encountering additional tracks beyond the given list / number is not
            considered an error
        """
        assert (expected_count is not None or expected_names)
        if expected_count is not None and expected_names:
            assert (expected_count == len(expected_names))
        if expected_count is None:
            expected_count = len(expected_names)

        tracks = self._find_tracks()

        if not allow_additional_tracks:
            self.expect_eq(len(tracks), expected_count, "# of tracks matches {}".format(expected_count))
        else:
            self.expect_true(len(tracks) >= expected_count, "# of tracks is at least {}".format(expected_count))

        names_found = 0
        if expected_names:
            logging.info("Matching visible tracks against name list ({})".format(expected_names))
            for name in expected_names:
                found = False
                for track in tracks:
                    track_name = track.texts()[0]
                    if self._match(name, track_name):
                        found = True
                        names_found += 1
                        break
                self.expect_true(found, "Found a match for track name '{}'".format(str(name)))

        if expected_names and not allow_additional_tracks:
            self.expect_eq(names_found, expected_count, "No additional tracks are found")

    @staticmethod
    def _match(expected_name: str or Iterable[str], found_name: str) -> bool:
        if isinstance(expected_name, str):
            return fnmatch(found_name, expected_name)
        else:
            for option in expected_name:
                if fnmatch(found_name, option):
                    return True
        return False


class CollapsingTrackBase(CaptureWindowE2ETestCaseBase):
    """
    Clicks on a track's triangle toggle and compares the track's height by calling `_verify_height`.
    """

    def _execute(self, expected_name: str):
        """
        :param expected_name: The exact name of the track to be collapsed. "*" is allowed as placeholder.
        """
        assert (expected_name != "")

        tracks = self._find_tracks(expected_name)

        self.expect_true(len(tracks) == 1, 'Found track {}'.format(expected_name))
        track = Track(tracks[0])

        prev_height = track.container.rectangle().height()
        triangle_toggle = track.triangle_toggle
        # TODO(b/184237564): Very short mouse clicks (within one frame) will be ignored. Thus we need expand
        #  the duration of the click explicitly.
        triangle_toggle.click_input(button_up=False)
        time.sleep(0.1)
        triangle_toggle.click_input(button_down=False)
        post_height = track.container.rectangle().height()
        self._verify_height(prev_height, post_height, expected_name)

    def _verify_height(self, prev_click_height: int, post_click_height: int, track_name: str):
        raise NotImplementedError("Implementation required")


class ExpandTrack(CollapsingTrackBase):
    """
    Click on a collapsed track's triangle toggle, verify that the height increased.
    """

    def _verify_height(self, prev_click_height: int, post_click_height: int, track_name: str):
        self.expect_true(prev_click_height < post_click_height, "Expanding track '{}' changed its height from {:d} to "
                                                                "{:d}".format(
            track_name, prev_click_height, post_click_height))


class CollapseTrack(CollapsingTrackBase):
    """
    Click on an expanded track's triangle toggle, verify that the height decreased.
    """

    def _verify_height(self, prev_click_height: int, post_click_height: int, track_name: str):
        self.expect_true(prev_click_height > post_click_height,
                         "Collapsing track '{}' changed its height from {:d} to {:d}".format(
                             track_name, prev_click_height, post_click_height))


class FilterTracks(CaptureWindowE2ETestCaseBase):
    """
    Set a filter in the capture tab, and verify either the amount of visible tracks or their names
    """

    def _execute(self, filter_string: str = "", expected_count: int = None, expected_names: List[str] = None,
                 allow_additional_tracks=False):
        """
        See MatchTracks._execute() for documentation.
        """
        toolbar = self.find_control("ToolBar", "CaptureToolBar")
        track_filter = self.find_control("Edit", "FilterTracks", parent=toolbar)

        logging.info("Setting track filter text: '{}'".format(filter_string))
        track_filter.set_focus()
        track_filter.set_edit_text('')
        # Using send_keys instead of set_edit_text directly because set_edit_text ignores the wait timings...
        keyboard.send_keys(filter_string)

        # Verify by re-using a MatchTracks Fragment
        match_tracks = MatchTracks(expected_count=expected_count, expected_names=expected_names,
                                   allow_additional_tracks=allow_additional_tracks)
        match_tracks.execute(self.suite)


class Capture(E2ETestCase):
    def _show_capture_window(self):
        logging.info('Showing capture window')
        self.find_control("TabItem", "Capture").click_input()

    def _set_capture_options(self, collect_thread_states: bool, collect_system_memory_usage: bool):
        capture_tab = self.find_control('Group', "CaptureTab")

        logging.info('Opening "Capture Options" dialog')
        capture_options_button = self.find_control('Button', 'Capture Options', parent=capture_tab)
        capture_options_button.click_input()

        capture_options_dialog = self.find_control('Window', 'Capture Options')

        collect_thread_states_checkbox = self.find_control('CheckBox', 'Collect thread states',
                                                           parent=capture_options_dialog)
        if collect_thread_states_checkbox.get_toggle_state() != collect_thread_states:
            logging.info('Toggling "Collect thread states" checkbox')
            collect_thread_states_checkbox.click_input()

        collect_system_memory_usage_checkbox = self.find_control('CheckBox',
                                                                 'Collect system-wide memory usage information',
                                                                 parent=capture_options_dialog)
        if collect_system_memory_usage_checkbox.get_toggle_state() != collect_system_memory_usage:
            logging.info('Toggling "Collect system-wide memory usage information" checkbox')
            collect_system_memory_usage_checkbox.click_input()

        logging.info('Saving "Capture Options"')
        self.find_control('Button', 'OK', parent=capture_options_dialog).click_input()

    def _take_capture(self, length_in_seconds: int):
        capture_tab = self.find_control('Group', "CaptureTab")
        toggle_capture_button = self.find_control('Button', 'Toggle Capture', parent=capture_tab)

        logging.info('Starting to capture for {} seconds'.format(length_in_seconds))
        toggle_capture_button.click_input()
        time.sleep(length_in_seconds)
        logging.info('Stopping capture')
        toggle_capture_button.click_input()
        self._wait_for_capture_completion()

    def _verify_existence_of_tracks(self):
        logging.info("Verifying existence of at least one track...")
        MatchTracks(expected_count=1, allow_additional_tracks=True).execute(self.suite)

    def _wait_for_capture_completion(self):
        logging.info("Waiting for capture to finalize...")
        wait_for_condition(lambda: self.find_control('Window', 'Finalizing capture',
                                                     recurse=False, raise_on_failure=False) is None, max_seconds=120)
        logging.info("Capturing finished")

    def _execute(self, length_in_seconds: int = 5, collect_thread_states: bool = False,
                 collect_system_memory_usage: bool = False):
        self._show_capture_window()
        self._set_capture_options(collect_thread_states, collect_system_memory_usage)
        self._take_capture(length_in_seconds)
        self._verify_existence_of_tracks()


class CheckThreadStates(CaptureWindowE2ETestCaseBase):
    def _execute(self, track_name_filter: str, expect_exists: bool = True):
        tracks = self._find_tracks(track_name_filter)
        self.expect_true(len(tracks) > 0, 'Found tracks matching {}'.format(track_name_filter))
        logging.info('Checking for thread states in {} tracks'.format(len(tracks)))
        for track in tracks:
            track = Track(track)
            if expect_exists:
                self.expect_true(track.thread_states is not None, 'Track {} has a thread state pane'.format(track.name))
            else:
                self.expect_true(track.thread_states is None, 'Track {} has a no thread state pane'.format(track.name))


class CheckTimers(CaptureWindowE2ETestCaseBase):
    def _execute(self, track_name_filter: str, expect_exists: bool = True, recursive: bool = False):
        tracks = self._find_tracks(track_name_filter, recursive)
        self.expect_true(len(tracks) > 0, 'Found tracks matching "{}"'.format(track_name_filter))
        logging.info('Checking for timers in {} tracks'.format(len(tracks)))
        for track in tracks:
            track = Track(track)
            if expect_exists:
                self.expect_true(track.timers is not None, 'Track "{}" has timers pane'.format(track.name))
            else:
                self.expect_true(track.timers is None, 'Track "{}" has no timers pane'.format(track.name))


class CheckCallstacks(CaptureWindowE2ETestCaseBase):
    def _execute(self, track_name_filter: str, expect_exists: bool = True):
        tracks = self._find_tracks(track_name_filter)
        self.expect_true(len(tracks) > 0, 'Found tracks matching "{}"'.format(track_name_filter))
        logging.info('Checking for callstacks pane in {} tracks'.format(len(tracks)))
        for track in tracks:
            track = Track(track)
            if expect_exists:
                self.expect_true(track.callstacks is not None, 'Track "{}" has callstacks pane'.format(track.name))
            else:
                self.expect_true(track.callstacks is None, 'Track "{}" has no callstacks pane'.format(track.name))


class SetAndCheckMemorySamplingPeriod(E2ETestCase):
    # TODO(http://b/186098691): Move the capture options dialog utilities to a common base class.

    def _show_capture_options_dialog(self):
        logging.info('Opening "Capture Options" dialog')
        capture_tab = self.find_control('Group', 'CaptureTab')
        capture_options_button = self.find_control('Button', 'Capture Options', parent=capture_tab)
        capture_options_button.click_input()

    def _enable_collect_system_memory_usage(self):
        logging.info('Selecting "Collect system-wide memory usage information" checkbox')
        collect_system_memory_usage_checkbox = self.find_control(
            'CheckBox', 'Collect system-wide memory usage information',
            parent=self.find_control('Window', 'Capture Options'))
        if not collect_system_memory_usage_checkbox.get_toggle_state():
            collect_system_memory_usage_checkbox.click_input()

    def _close_capture_options_dialog(self):
        logging.info('Saving "Capture Options"')
        self.find_control(
            'Button', 'OK', parent=self.find_control('Window', 'Capture Options')).click_input()

    def _execute(self, memory_sampling_period: str):
        self._show_capture_options_dialog()
        self._enable_collect_system_memory_usage()
        memory_sampling_period_edit = self.find_control('Edit', 'MemorySamplingPeriodEdit')

        DEFAULT_SAMPLING_PERIOD = "10"
        if not memory_sampling_period:
            # If the user clean the input, sampling period should be reset to the default value
            expected = DEFAULT_SAMPLING_PERIOD
        elif (not memory_sampling_period.isdecimal()) or (not memory_sampling_period.lstrip('0')):
            # If the input contains invalid characters (e.g., letters) or its value equals to 0,
            # sampling period should keep the same as the previous value.
            expected = memory_sampling_period_edit.texts()[0]
        else:
            expected = memory_sampling_period.lstrip('0')

        logging.info('Entering "{}" in "Sampling period (ms)" edit'.format(memory_sampling_period))
        memory_sampling_period_edit.set_edit_text(memory_sampling_period)

        # Close and reopen capture options dialog to validate the setting of sampling period.
        self._close_capture_options_dialog()
        self._show_capture_options_dialog()
        logging.info('Validating the value in "Sampling period (ms)" edit')
        memory_sampling_period_edit = self.find_control('Edit', 'MemorySamplingPeriodEdit')
        result = memory_sampling_period_edit.texts()[0]
        self.expect_true(result == expected,
                         'Memory sampling period is set to "{}" while it should be "{}"'.format(result, expected))
        self._close_capture_options_dialog()
