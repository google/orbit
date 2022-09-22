"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging
import random
import time
from fnmatch import fnmatch

from typing import Tuple, List, Iterable

from core.common_controls import Track, Table
from core.orbit_e2e import E2ETestCase, E2ETestSuite, wait_for_condition
from pywinauto import mouse, keyboard
from pywinauto.base_wrapper import BaseWrapper
from pywinauto.keyboard import send_keys


class CaptureWindowE2ETestCaseBase(E2ETestCase):
    """
    Base class for fragments interacting with the capture window, provides common functionality.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._time_graph = None

    def execute(self, suite: E2ETestSuite):
        self._time_graph = self.find_control('Image', name='TimeGraph', parent=suite.top_window())
        self._track_container = self.find_control('Pane',
                                                  name='TrackContainer',
                                                  parent=self._time_graph)
        super().execute(suite=suite)

    def _find_tracks(self, name_filter: str = None, recursive: bool = False):
        if not recursive:
            tracks = self._track_container.children()
            if name_filter is not None:
                tracks = filter(lambda track: fnmatch(track.texts()[0], name_filter), tracks)
            return list(tracks)

        work_list = self._track_container.children()
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

    def _execute(self,
                 track_index: int = 0,
                 check_selection_before: bool = False,
                 expect_failure: bool = False):
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
        self.expect_true(not self._find_tracks()[index].has_keyboard_focus(),
                         "Track is no longer selected")
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
        self.expect_eq(
            index, expected_new_index % track_count,
            "Expected track index {} after reordering, got {}".format(expected_new_index, index))


class VerifyTracksExist(CaptureWindowE2ETestCaseBase):
    """
    Checks if one or multiple specified tracks are currently visible in the capture window.
    """

    def _execute(self,
                 track_names: str or List[str or Iterable[str]] = None,
                 allow_duplicates=False):
        """
        :param track_names: List of (partial) matching names of tracks to be visible, or a single track name.
            Names can be a partial match with wildcards (using fnmatch). Each entry in this list can either
            be a string or a tuple of strings. If an entry is a tuple of strings, the test expects at least on of the
            names to be matched (i.e. this is an "or" condition on multiple possible track names)
        :param allow_duplicates: If False, it is considered an error if more than one track matches a name
        """
        tracks = self._find_tracks()

        logging.info("Matching visible tracks against name list ({})".format(track_names))
        if isinstance(track_names, str):
            track_names = [track_names]

        for name in track_names:
            found = 0
            for track in tracks:
                track_name = track.texts()[0]
                if self._match(name, track_name):
                    found += 1
            if allow_duplicates:
                self.expect_true(found > 0, "Found a match for track name '{}'".format(str(name)))
            else:
                self.expect_true(found == 1,
                                 "Found exactly one match for track name '{}'".format(str(name)))

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

    def _execute(self, expected_name: str, recursive: bool = False):
        """
        :param expected_name: The exact name of the track to be collapsed. "*" is allowed as placeholder.
        """
        assert (expected_name != "")

        tracks = self._find_tracks(expected_name, recursive=recursive)

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
        self.expect_true(
            prev_click_height < post_click_height,
            "Expanding track '{}' changed its height from {:d} to "
            "{:d}".format(track_name, prev_click_height, post_click_height))


class CollapseTrack(CollapsingTrackBase):
    """
    Click on an expanded track's triangle toggle, verify that the height decreased.
    """

    def _verify_height(self, prev_click_height: int, post_click_height: int, track_name: str):
        self.expect_true(
            prev_click_height > post_click_height,
            "Collapsing track '{}' changed its height from {:d} to {:d}".format(
                track_name, prev_click_height, post_click_height))


class ToggleCollapsedStateOfAllTracks(CaptureWindowE2ETestCaseBase):
    """
    Clicks on the triangle toggles of all tracks.
    """

    def _execute(self):
        tracks = self._find_tracks("*")
        for track in tracks:
            track = Track(track)
            triangle_toggle = track.triangle_toggle
            triangle_toggle.click_input(button_up=False)
            time.sleep(0.1)
            triangle_toggle.click_input(button_down=False)


class FilterTracks(CaptureWindowE2ETestCaseBase):
    """
    Set a filter in the capture tab.
    """

    def _execute(self,
                 filter_string: str = "",
                 expected_track_count=None,
                 expected_minimum_track_count=None,
                 expected_maximum_track_count=None):
        """
        :param filter_string: The string to be entered in the filter edit. Note that pywinauto's `keyboard.send_keys` is
            used, so certain special characters need to be escaped by enclosing them in {}.
        :param expected_track_count: If not None, this test will verify that the number of tracks after filtering is exactly this.
        :param expected_minimum_track_count: If not None, this test will verify that the number of tracks after filtering is at least this.
        :param expected_maximum_track_count: If not None, this test will verify that the number of tracks after filtering is at most this.
        """
        toolbar = self.find_control("ToolBar", "CaptureToolBar")
        track_filter = self.find_control("Edit", "FilterTracks", parent=toolbar)

        logging.info("Setting track filter text: '{}'".format(filter_string))
        track_filter.set_focus()
        track_filter.set_edit_text('')
        # Using send_keys instead of set_edit_text directly because set_edit_text ignores the wait timings...
        keyboard.send_keys(filter_string)

        if expected_track_count is not None:
            wait_for_condition(lambda: len(self._find_tracks()) == expected_track_count)

        if expected_minimum_track_count is not None:
            wait_for_condition(lambda: len(self._find_tracks()) >= expected_minimum_track_count)

        if expected_maximum_track_count is not None:
            wait_for_condition(lambda: len(self._find_tracks()) <= expected_maximum_track_count)


class ZoomOut(CaptureWindowE2ETestCaseBase):
    """
    Zooms the capture view out all the way so that the entire capture is visible.
    """

    def _execute(self):
        logging.info('Zooming out the capture window')
        self._time_graph.click_input()
        # This should be enough so that we can zoom all the way out from any zoom level with any capture duration.
        for i in range(200):
            send_keys("s")


class CaptureE2ETestCaseBase(E2ETestCase):

    def _show_capture_window(self):
        logging.info('Showing capture window')
        self.find_control("TabItem", "Capture").click_input()

    def _show_and_get_capture_options_dialog(self):
        logging.info('Opening "Capture Options" dialog')
        capture_tab = self.find_control('Group', 'CaptureTab')
        capture_options_button = self.find_control('Button', 'Capture Options', parent=capture_tab)
        capture_options_button.click_input()
        wait_for_condition(lambda: self.find_control(
            'Window', 'Capture Options', recurse=False, raise_on_failure=False) is not None,
                           max_seconds=40)
        return self.find_control('Window', 'Capture Options')

    def _close_capture_options_dialog(self):
        logging.info('Saving "Capture Options"')
        self.find_control('Button', 'OK',
                          parent=self.find_control('Window', 'Capture Options')).click_input()

    def _set_collect_thread_states_capture_option(self, collect_thread_states: bool,
                                                  capture_options_dialog):
        collect_thread_states_checkbox = self.find_control('CheckBox',
                                                           'CollectThreadStatesCheckBox',
                                                           parent=capture_options_dialog)
        if collect_thread_states_checkbox.get_toggle_state() != collect_thread_states:
            logging.info('Toggling "Collect thread states" checkbox to {}.', collect_thread_states)
            collect_thread_states_checkbox.click_input()

    def _set_enable_auto_frame_track_capture_option(self, enable_auto_frame_track: bool,
                                                    capture_options_dialog):
        enable_auto_frame_track_checkbox = self.find_control('CheckBox',
                                                             'AutoFrameTrackCheckBox',
                                                             parent=capture_options_dialog)
        if enable_auto_frame_track_checkbox.get_toggle_state() != enable_auto_frame_track:
            logging.info('Toggling "Auto Frame Track" checkbox to {}.', enable_auto_frame_track)
            enable_auto_frame_track_checkbox.click_input()

    def _set_collect_system_memory_usage_capture_option(self, collect_system_memory_usage: bool,
                                                        capture_options_dialog):
        collect_system_memory_usage_checkbox = self.find_control('CheckBox',
                                                                 'CollectMemoryInfoCheckBox',
                                                                 parent=capture_options_dialog)
        if collect_system_memory_usage_checkbox.get_toggle_state() != collect_system_memory_usage:
            logging.info(
                'Toggling "Collect memory usage and page faults information" checkbox to {}.',
                collect_system_memory_usage)
            collect_system_memory_usage_checkbox.click_input()

    def _set_dynamic_instrumentation_method_capture_option(self, user_space_instrumentation: bool,
                                                           capture_options_dialog):
        if user_space_instrumentation:
            logging.info('Setting dynamic instrumentation method to "Orbit".')
            user_space_radio_button = self.find_control('RadioButton',
                                                        'UserSpaceInstrumentationRadioButton',
                                                        parent=capture_options_dialog)
            user_space_radio_button.click_input()
        else:
            logging.info('Setting dynamic instrumentation method to "Kernel (Uprobes)".')
            uprobes_radio_button = self.find_control('RadioButton',
                                                     'UprobesRadioButton',
                                                     parent=capture_options_dialog)
            uprobes_radio_button.click_input()

    def _set_manual_instrumentation_capture_option(self, manual_instrumentation: bool,
                                                   capture_options_dialog):
        manual_instrumentation_checkbox = self.find_control('CheckBox',
                                                            'ApiCheckBox',
                                                            parent=capture_options_dialog)
        if manual_instrumentation_checkbox.get_toggle_state() != manual_instrumentation:
            logging.info('Toggling "Enable Orbit Api in target" checkbox to {}.',
                         manual_instrumentation)
            manual_instrumentation_checkbox.click_input()

    def _set_unwinding_method_capture_option(self, frame_pointer_unwinding: bool,
                                             capture_options_dialog):
        if frame_pointer_unwinding:
            logging.info('Setting sampling method to "Frame pointers".')
            frame_pointer_unwinding_radio_button = self.find_control('RadioButton',
                                                                     'FramePointerRadioButton',
                                                                     parent=capture_options_dialog)
            frame_pointer_unwinding_radio_button.click_input()
        else:
            logging.info('Setting sampling method to "DWARF".')
            dwarf_unwinding_radio_button = self.find_control('RadioButton',
                                                             'DWARFRadioButton',
                                                             parent=capture_options_dialog)
            dwarf_unwinding_radio_button.click_input()

    def _set_sampling_period_ms_capture_option(self, sampling_period_ms: float,
                                               capture_options_dialog):
        logging.info('Setting sampling period to {}.'.format(sampling_period_ms))
        sampling_period_spin_box = self.find_control('Spinner',
                                                     'SamplingPeriodMsDoubleSpinBox',
                                                     parent=capture_options_dialog)
        sampling_period_spin_box.set_focus()
        sampling_period_ms_str = str(sampling_period_ms)
        current_sampling_period_ms_str = sampling_period_spin_box.texts()[0]

        # Check if Qt is expecting german floating points
        if ',' in current_sampling_period_ms_str:
            sampling_period_ms_str = sampling_period_ms_str.replace('.', ',')

        # Select all, type in new sampling period. There seemed to be a timing issue that's why the sleep was added.
        time.sleep(0.5)
        send_keys("^a")
        send_keys(sampling_period_ms_str)

        if sampling_period_spin_box.texts()[0] != sampling_period_ms_str:
            raise RuntimeError('Setting sampling period failed. Expected "{}", but got "{}'.format(
                sampling_period_ms_str,
                sampling_period_spin_box.texts()[0]))

    def _set_capture_options(self, collect_thread_states: bool, collect_system_memory_usage: bool,
                             user_space_instrumentation: bool, manual_instrumentation: bool,
                             frame_pointer_unwinding: bool, sampling_period_ms: float):
        capture_options_dialog = self._show_and_get_capture_options_dialog()

        self._set_collect_thread_states_capture_option(collect_thread_states,
                                                       capture_options_dialog)
        self._set_collect_system_memory_usage_capture_option(collect_system_memory_usage,
                                                             capture_options_dialog)
        self._set_dynamic_instrumentation_method_capture_option(user_space_instrumentation,
                                                                capture_options_dialog)
        self._set_manual_instrumentation_capture_option(manual_instrumentation,
                                                        capture_options_dialog)
        self._set_unwinding_method_capture_option(frame_pointer_unwinding, capture_options_dialog)
        self._set_sampling_period_ms_capture_option(sampling_period_ms, capture_options_dialog)

        self._close_capture_options_dialog()

    def _set_up_and_start_capture(self, collect_thread_states: bool,
                                  collect_system_memory_usage: bool,
                                  user_space_instrumentation: bool, manual_instrumentation: bool,
                                  frame_pointer_unwinding: bool, sampling_period_ms: float,
                                  toggle_capture_button):
        self._show_capture_window()
        self._set_capture_options(collect_thread_states, collect_system_memory_usage,
                                  user_space_instrumentation, manual_instrumentation,
                                  frame_pointer_unwinding, sampling_period_ms)
        logging.info('Starting to capture')
        toggle_capture_button.click_input()

    def _verify_existence_of_timeline(self):
        logging.info("Verifying existence of the timeline...")
        timeline = self.find_control('Pane', name='Timeline')
        self.expect_true(len(timeline.children()) == 0, 'Timeline exists and has no children')

    def _verify_existence_of_tracks(self):
        logging.info("Verifying existence of at least one track...")
        track_container = self.find_control('Pane', name='TrackContainer')
        self.expect_true(len(track_container.children()),
                         'Track container exists and has at least one child')

    def _verify_capture(self):
        self._verify_existence_of_timeline()
        self._verify_existence_of_tracks()


class Capture(CaptureE2ETestCaseBase):

    def _wait_for_capture_completion(self):
        logging.info("Waiting for capture to finalize...")
        wait_for_condition(lambda: self.find_control(
            'Window', 'Finalizing capture', recurse=False, raise_on_failure=False) is None,
                           max_seconds=120)
        logging.info("Capturing finished")

    def _stop_capture(self, toggle_capture_button):
        logging.info('Stopping capture')
        toggle_capture_button.click_input()
        self._wait_for_capture_completion()

    def _execute(self,
                 length_in_seconds: int = 5,
                 collect_thread_states: bool = False,
                 collect_system_memory_usage: bool = False,
                 user_space_instrumentation: bool = False,
                 manual_instrumentation: bool = False,
                 frame_pointer_unwinding: bool = False,
                 sampling_period_ms: float = 1.0):
        capture_tab = self.find_control('Group', "CaptureTab")
        toggle_capture_button = self.find_control('Button', 'Toggle Capture', parent=capture_tab)
        self._set_up_and_start_capture(collect_thread_states, collect_system_memory_usage,
                                       user_space_instrumentation, manual_instrumentation,
                                       frame_pointer_unwinding, sampling_period_ms,
                                       toggle_capture_button)
        logging.info('Capturing for {} seconds'.format(length_in_seconds))
        time.sleep(length_in_seconds)
        self._stop_capture(toggle_capture_button)
        self._verify_capture()


class CaptureAndWaitForInterruptedWarning(CaptureE2ETestCaseBase):

    def _wait_and_verify_capture_interrupted(self):
        logging.info("Verifying that the capture was stopped by OrbitService")
        wait_for_condition(lambda: self.find_control(
            'Window', 'Capture interrupted', recurse=False, raise_on_failure=False) is not None,
                           max_seconds=300)
        dialog = self.find_control('Window', 'Capture interrupted')
        self.find_control('Button', 'OK', parent=dialog).click_input()

    def _execute(self,
                 collect_thread_states: bool = False,
                 collect_system_memory_usage: bool = False,
                 user_space_instrumentation: bool = False,
                 manual_instrumentation: bool = False):
        capture_tab = self.find_control('Group', "CaptureTab")
        toggle_capture_button = self.find_control('Button', 'Toggle Capture', parent=capture_tab)
        self._set_up_and_start_capture(collect_thread_states, collect_system_memory_usage,
                                       user_space_instrumentation, manual_instrumentation,
                                       toggle_capture_button)
        self._wait_and_verify_capture_interrupted()
        self._verify_capture()


class CaptureRepeatedly(E2ETestCase):
    """
    This test case simulates a user quickly starting and stopping captures by "smashing" the F5 key. Note that the
    number of captures actually taken is going to be much lower than the number of times F5 was pressed, because
    starting and stopping a capture takes some times.
    The test does not verify any behavior, but it will fail if Orbit crashes.
    """

    def _stop_capture_if_necessary(self, capture_tab):
        logging.info('Querying if a capture is still running')
        capture_options_button = self.find_control('Button', 'Capture Options', parent=capture_tab)
        if not capture_options_button.is_enabled():
            logging.info('A capture is still running: stopping it')
            toggle_capture_button = self.find_control('Button',
                                                      'Toggle Capture',
                                                      parent=capture_tab)
            toggle_capture_button.click_input()

    def _execute(self, number_of_f5_presses: int):
        capture_tab = self.find_control('Group', "CaptureTab")
        for i in range(number_of_f5_presses):
            logging.info('Pressing F5 the {}-th time'.format(i + 1))
            send_keys('{F5}')
            time.sleep(random.uniform(0.05, 0.1))

        logging.info('Checking that Orbit is still running')
        self.suite.top_window(force_update=True)

        self._stop_capture_if_necessary(capture_tab)

        capture_options_button = self.find_control('Button', 'Capture Options', parent=capture_tab)
        wait_for_condition(lambda: capture_options_button.is_enabled() is True, max_seconds=30)


class CheckThreadStates(CaptureWindowE2ETestCaseBase):

    def _execute(self, track_name_filter: str, expect_exists: bool = True):
        tracks = self._find_tracks(track_name_filter)
        self.expect_true(len(tracks) > 0, 'Found tracks matching {}'.format(track_name_filter))
        logging.info('Checking for thread states in {} tracks'.format(len(tracks)))
        for track in tracks:
            track = Track(track)
            if expect_exists:
                self.expect_true(track.thread_states is not None,
                                 'Track {} has a thread state pane'.format(track.name))
            else:
                self.expect_true(track.thread_states is None,
                                 'Track {} has a no thread state pane'.format(track.name))


# TODO(b/239396452): Consider filtering the tracks before looking for matching ones in order to avoid failures related
#  to a track being too far down and hence off-screen.
class CheckTimers(CaptureWindowE2ETestCaseBase):

    def _check_all_tracks(self, tracks, expect_exists):
        for track in tracks:
            track = Track(track)
            if expect_exists:
                self.expect_true(track.timers is not None,
                                 'Track "{}" has timers pane'.format(track.name))
            else:
                self.expect_true(track.timers is None,
                                 'Track "{}" has no timers pane'.format(track.name))

    def _check_at_least_one_track(self, track_name_filter, tracks, expect_exists):
        satisfying_count = 0
        for track in tracks:
            track = Track(track)
            if expect_exists and track.timers is not None:
                satisfying_count += 1
            elif not expect_exists and track.timers is None:
                satisfying_count += 1
        if expect_exists:
            self.expect_true(
                satisfying_count > 0,
                'At least one track matching "{}" has timers pane'.format(track_name_filter))
        else:
            self.expect_true(
                satisfying_count > 0,
                'At least one track matching "{}" has no timers pane'.format(track_name_filter))

    def _execute(self,
                 track_name_filter: str,
                 expect_exists: bool = True,
                 require_all: bool = True,
                 recursive: bool = False):
        tracks = self._find_tracks(track_name_filter, recursive)
        self.expect_true(len(tracks) > 0, 'Found tracks matching "{}"'.format(track_name_filter))
        logging.info('Checking timers in {} tracks'.format(len(tracks)))
        if require_all:
            self._check_all_tracks(tracks, expect_exists)
        else:
            self._check_at_least_one_track(track_name_filter, tracks, expect_exists)


class CheckCallstacks(CaptureWindowE2ETestCaseBase):

    def _execute(self, track_name_filter: str, expect_exists: bool = True):
        tracks = self._find_tracks(track_name_filter)
        self.expect_true(len(tracks) > 0, 'Found tracks matching "{}"'.format(track_name_filter))
        logging.info('Checking for callstacks pane in {} tracks'.format(len(tracks)))
        for track in tracks:
            track = Track(track)
            if expect_exists:
                self.expect_true(track.callstacks is not None,
                                 'Track "{}" has callstacks pane'.format(track.name))
            else:
                self.expect_true(track.callstacks is None,
                                 'Track "{}" has no callstacks pane'.format(track.name))


class SelectAllCallstacksFromTrack(CaptureWindowE2ETestCaseBase):
    """
    Select all currently visible callstacks (based on the current visible section of the capture) from the track with
    the specified name.
    """

    def _execute(self, track_name_filter: str):
        tracks = self._find_tracks(track_name_filter)
        self.expect_true(
            len(tracks) == 1, 'Found exactly one track matching "{}"'.format(track_name_filter))
        track = Track(tracks[0])
        self.expect_true(track.callstacks is not None,
                         'Track "{}" has callstacks pane'.format(track.name))

        logging.info('Selecting all visible callstacks in track "{}"'.format(track.name))
        callstacks = track.callstacks
        rect = callstacks.rectangle()
        callstacks.drag_mouse_input(src=(rect.left, rect.top + 5), dst=(rect.right, rect.top + 5))

        self.expect_true(
            self.find_control('TabItem', 'Sampling (selection)').is_enabled(),
            "'Sampling (selection)' tab is enabled")
        self.expect_true(
            self.find_control('TabItem', 'Top-Down (selection)').is_enabled(),
            "'Top-Down (selection)' tab is enabled")
        self.expect_true(
            self.find_control('TabItem', 'Bottom-Up (selection)').is_enabled(),
            "'Bottom-Up (selection)' tab is enabled")
        logging.info("Verified that '(selection)' tabs are enabled")


class SetEnableAutoFrameTrack(CaptureE2ETestCaseBase):
    """
    Set the auto frame track setting option. This is a public method and not a parameter in Capture()
    because since the already added FrameTrack won't be removed from the current session, the E2E
    tests will need to end and start a new session before taking the capture.
    """

    def _execute(self, enable_auto_frame_track: bool):
        capture_options_dialog = self._show_and_get_capture_options_dialog()
        self._set_enable_auto_frame_track_capture_option(enable_auto_frame_track,
                                                         capture_options_dialog)
        self._close_capture_options_dialog()


class SetAndCheckMemorySamplingPeriod(CaptureE2ETestCaseBase):

    def _execute(self, memory_sampling_period: str):
        capture_options_dialog = self._show_and_get_capture_options_dialog()
        self._set_collect_system_memory_usage_capture_option(True, capture_options_dialog)
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
        self._show_and_get_capture_options_dialog()
        logging.info('Validating the value in "Sampling period (ms)" edit')
        memory_sampling_period_edit = self.find_control('Edit', 'MemorySamplingPeriodEdit')
        result = memory_sampling_period_edit.texts()[0]
        self.expect_true(
            result == expected,
            'Memory sampling period is set to "{}" while it should be "{}"'.format(
                result, expected))
        self._close_capture_options_dialog()


class VerifyTracksDoNotExist(CaptureWindowE2ETestCaseBase):
    """
    Checks if one or multiple specified tracks are NOT visible in the capture window.
    """

    def _execute(self, track_names: str or Iterable):
        """
        :param track_names: List of track names or a single track name. May contain wildcards (using fnmatch as
        matching implementation).
        """
        if isinstance(track_names, str):
            self.expect_true(
                len(self._find_tracks(track_names)) == 0,
                'Track {} was found, but should not have been'.format(track_names))
        else:
            for track_name in track_names:
                self.expect_true(
                    len(self._find_tracks(track_name)) == 0,
                    'Track {} was found, but should not have been'.format(track_name))
            toggle_button = self.find_control('CheckBox', 'Track Configuration Pane')
            toggle_button.click_input()


class ToggleTrackTypeVisibility(CaptureWindowE2ETestCaseBase):
    """
    Opens the track configuration pane, changes the visibility of certain track types. This does not verify the
    result itself - use MatchTracks() for this.
    """

    def _execute(self, track_type: str):
        control_pane = self._show_and_find_control_pane()
        table = self.find_control("Table", "TrackTypeVisibility", parent=control_pane)
        table_obj = Table(table)
        row = table_obj.find_first_item_row(track_type, 1)
        table_obj.get_item_at(row, 0).click_input(coords=(10, 10))

    def _show_and_find_control_pane(self):
        control_pane = self.find_control("Group", "TrackConfigurationPane", raise_on_failure=False)
        if control_pane is None:
            toggle_button = self.find_control('CheckBox', 'Track Configuration Pane')
            toggle_button.click_input()
        control_pane = self.find_control("Group", "TrackConfigurationPane")
        return control_pane
