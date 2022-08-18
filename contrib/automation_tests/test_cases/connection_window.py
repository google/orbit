"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging
import os
import tempfile
import shutil

from typing import Iterable
from pywinauto.application import Application
from pywinauto.keyboard import send_keys

from core.orbit_e2e import E2ETestCase, OrbitE2EError, wait_for_condition
from core.common_controls import get_tooltip


def _wait_for_main_window(application: Application, timeout=30):
    wait_for_condition(lambda: application.top_window().class_name() == "OrbitMainWindow",
                       max_seconds=timeout)
    application.top_window().set_focus()


def _wait_for_connection_window(application: Application):
    wait_for_condition(
        lambda: application.top_window().class_name() == "orbit_session_setup::SessionSetupDialog",
        max_seconds=30)
    application.top_window().set_focus()


def _get_number_of_instances_in_list(test_case: E2ETestCase) -> int:
    instance_list = test_case.find_control('Table', 'InstanceList')
    instance_count = instance_list.item_count()
    logging.info('Found %s rows in the instance list', instance_count)
    return instance_count


class LoadCapture(E2ETestCase):
    """
    Opens a given capture file. The argument `capture_file_path` specifies the path to the capture file.
    The given path can be either
      (*) absolute, in which case it is taken as is, or
      (*) relative, in which case it is interpreted relative to `automation_tests` directory.
    If `expect_fail` is true, the test checks for an error dialog after loading the capture and closes the dialog.
    It raises an exception if the capture does not exists. Also waits until the capture is loaded.
    """

    def _execute(self, capture_file_path: str, expect_fail=False):
        # Lets ensure that we are actually in the connection window, as this test might also be executed after the
        # main window has been opened.
        _wait_for_connection_window(self.suite.application)
        self.suite.top_window(force_update=True)
        logging.info('Starting to load a capture.')
        connect_radio = self.find_control('RadioButton', 'LoadCapture')
        connect_radio.click_input()

        others_button = self.find_control('Button', '...')
        others_button.click_input()

        wait_for_condition(lambda: self.find_control('Edit', 'File name:') is not None,
                           max_seconds=120)
        file_name_edit = self.find_control('Edit', 'File name:')

        if not os.path.isabs(capture_file_path):
            # Copy the capture file into temporary location. We need write access to the file (see http://b/229836007)
            # and we don't have this for files in the `automation_tests` directory.
            automation_tests_dir = os.path.join(os.path.dirname(__file__), '..')
            src_path = os.path.join(automation_tests_dir, capture_file_path)
            dst_path = os.path.join(tempfile.gettempdir(), os.path.basename(capture_file_path))
            shutil.copyfile(src_path, dst_path)
            capture_file_path = dst_path

        file_name_edit.set_edit_text(capture_file_path)

        logging.info(f'Trying to load capture file: {capture_file_path}')
        start_session_button = self.find_control('Button', 'Start Session')
        start_session_button.click_input()

        # Unless the file does not exist, Orbit's main window will open next and contain the "Loading capture" dialog.
        _wait_for_main_window(self.suite.application)
        # As there is a new top window (Orbit main window), we need to update the top window.
        self.suite.top_window(force_update=True)

        logging.info("Waiting for capture to load...")
        wait_for_condition(
            lambda: self.find_control('Window', 'Loading capture', raise_on_failure=False) is None,
            max_seconds=120)
        logging.info("Capture Loading finished")

        if expect_fail:
            self.find_control('Window', 'Error while loading capture')
            ok_button = self.find_control('Button', 'OK')
            ok_button.click_input()


class LoadLatestCapture(E2ETestCase):
    """
    Opens the most recent capture that matches the first given `filter_strings`. The filter strings get processed in
    order, and the next filter string is used if no capture was found for a particular string. Also it waits until the
    capture is loaded.

    It raises an exception if no capture was found at all.
    """

    def _execute(self, filter_strings: str or Iterable[str]):
        if isinstance(filter_strings, str):
            filter_strings = [filter_strings]
        connect_radio = self.find_control('RadioButton', 'LoadCapture')
        connect_radio.click_input()

        filter_capture_files = self.find_control('Edit', 'FilterCaptureFiles')
        succeeded = False
        for filter_string in filter_strings:
            filter_capture_files.set_edit_text(filter_string)

            capture_file_list = self.find_control('Table', 'CaptureFileList')
            try:
                wait_for_condition(lambda: capture_file_list.item_count() > 0, 30)
            except OrbitE2EError:
                continue
            succeeded = True
            capture_file_list.children(control_type='DataItem')[0].double_click_input()

            # Unless the file does not exist, Orbit's main window will open and contain the "Loading capture" dialog.
            _wait_for_main_window(self.suite.application)
            # As there is a new top window (Orbit main window), we need to update the top window.
            self.suite.top_window(force_update=True)
            break

        if not succeeded:
            raise OrbitE2EError(f'Did not find any capture in filter strings: "{filter_strings}"')


class ConnectToStadiaInstance(E2ETestCase):
    """
    Connect to the first available stadia instance
    """

    def _execute(self):
        window = self.suite.top_window()

        logging.info('Start connecting to gamelet.')
        connect_radio = self.find_control('RadioButton', 'ConnectToStadia')
        connect_radio.click_input()

        # Wait for the list to be populated with more than 0 instances
        instance_list = self.find_control('Table', 'InstanceList')
        instance_list.click_input()
        wait_for_condition(lambda: _get_number_of_instances_in_list(self) > 0, max_seconds=100)
        self.expect_true(_get_number_of_instances_in_list(self) >= 1, 'Found at least one instance')

        instance_list = self.find_control('Table', 'InstanceList')
        instance_list.children(control_type='DataItem')[0].double_click_input()
        logging.info('Connecting to Instance, waiting for the process list...')

        # In the new UI, use small waits until the process list is active, and then some more for the
        # semi-transparent "loading" Overlay of the tables to disappear
        wait_for_condition(
            lambda: self.find_control('Custom', 'ProcessesFrame').is_enabled() is True, 50)
        wait_for_condition(lambda: self.find_control('Table', 'ProcessList').is_active(), 20)
        # This is a bit annoying, but since the overlay is invisible when loading is done, we need to check for
        # absence of the overlay... not sure if there is a better way
        wait_for_condition(lambda: self.find_control(
            'Group', 'ProcessListOverlay', raise_on_failure=False) is None)
        logging.info('Process list ready')


class DisconnectFromStadiaInstance(E2ETestCase):
    """
    Disconnect from the instance. Requires to be connected. Verify that there are instances after the disconnect.
    """

    def _execute(self):
        instance_list_overlay = self.find_control('Group', 'InstanceListOverlay')
        self.expect_true(instance_list_overlay.is_visible(), "Instance overlay is visible")
        disconnect_button = self.find_control('Button', 'Disconnect')
        logging.info(
            'Instance overlay visible and Disconnect button was found; performing disconnect from the instance'
        )
        disconnect_button.click_input()

        logging.info('Waiting for instance overlay to dissapear')
        wait_for_condition(lambda: self.find_control(
            'Group', 'InstanceListOverlay', raise_on_failure=False) is None)
        logging.info('Loading done, overlay is hidden')
        self.expect_true(_get_number_of_instances_in_list(self) >= 1, 'Found at least one instance')
        wait_for_condition(lambda: self.find_control(
            'Group', 'ProcessListOverlay', raise_on_failure=False) is None)


class RefreshStadiaInstanceList(E2ETestCase):
    """
    Click the refresh button and check that the loading overlay is displayed and the list contains
    items when the loading finished.
    """

    def _execute(self):
        refresh_button = self.find_control('Button', 'RefreshInstanceList')
        logging.info('Found instance list and refresh button, clicking refresh button')
        refresh_button.click_input()

        instance_list_overlay = self.find_control(None, 'InstanceListOverlay')
        self.expect_true(instance_list_overlay.is_visible(), "Instance overlay is visible")
        logging.info('Found InstanceListOverlay and its visible')

        wait_for_condition(
            lambda: self.find_control('Group', 'InstanceListOverlay', raise_on_failure=False) is
            None, 100)
        logging.info('Loading done, overlay is hidden')
        self.expect_true(_get_number_of_instances_in_list(self) >= 1, 'Found at least one instance')


class SelectProjectAndVerifyItHasAtLeastOneInstance(E2ETestCase):
    """
    Expand the project selection combo box and click the entry `project_name`. Also waits
    appropriately when loading takes place.
    """

    def _execute(self, project_name: str):
        logging.info('Select project with name ' + project_name)
        project_combo_box = self.find_control(name='ProjectSelectionComboBox')
        # wait until loading is done
        wait_for_condition(lambda: project_combo_box.is_enabled() is True, 25)
        project_combo_box.click_input()

        project = self.find_combo_box_item(text=project_name)
        logging.info('Expanded project combo box and found entry ' + project_name)
        project.click_input()
        # After project changing, loading takes place. Wait until the overlay is hidden
        wait_for_condition(
            lambda: self.find_control('Group', 'InstanceListOverlay', raise_on_failure=False) is
            None, 100)
        logging.info('Successfully selected project ' + project_name +
                     'and waited until loading is done')
        self.expect_true(_get_number_of_instances_in_list(self) >= 1, 'Found at least one instance')


class SelectNextProject(E2ETestCase):
    """
    Expand the project selection combo box and select the next project by pressing the down arrow
    and enter. Also waits appropriately when loading takes place.
    """

    def _execute(self):
        logging.info('Select next project')
        project_combo_box = self.find_control(name='ProjectSelectionComboBox')
        # wait until loading is done
        wait_for_condition(lambda: project_combo_box.is_enabled() is True, 25)
        project_combo_box.click_input()
        logging.info('Found and opened project combo box, sending down arrow and enter')
        send_keys('{DOWN}{ENTER}')
        # After project changing, loading takes place. Wait until the overlay is hidden
        wait_for_condition(
            lambda: self.find_control('Group', 'InstanceListOverlay', raise_on_failure=False) is
            None, 100)
        logging.info('Successfully selected next project')


class TestAllInstancesCheckbox(E2ETestCase):
    """
    This test expects that the "All Instances" check box is not checked at the start. The Test
    clicks the check box and expects that the number of instances stays the same or increases.
    Afterwards the checkbox is clicked again to reset the state.
    """

    def _execute(self):
        logging.info('Starting "All Instances" checkbox test.')
        # First wait until all loading is done.
        wait_for_condition(
            lambda: self.find_control('Group', 'InstanceListOverlay', raise_on_failure=False) is
            None, 100)
        check_box = self.find_control('CheckBox', 'AllInstancesCheckBox')
        logging.info('Found checkbox')

        instance_count = _get_number_of_instances_in_list(self)

        check_box.click_input()
        logging.info('Clicked All Instances check box, waiting until loading is done')
        wait_for_condition(
            lambda: self.find_control('Group', 'InstanceListOverlay', raise_on_failure=False) is
            None, 100)
        self.expect_true(instance_count <= _get_number_of_instances_in_list(self),
                         'Instance list contains at least same amount of instances as before.')
        logging.info('Loading successful, instance number increased')

        check_box.click_input()
        wait_for_condition(
            lambda: self.find_control('Group', 'InstanceListOverlay', raise_on_failure=False) is
            None, 100)
        self.expect_true(
            _get_number_of_instances_in_list(self) >= 1,
            'Instance list contains at least one instance')
        logging.info('Second click on check box successful')


class FilterAndSelectFirstProcess(E2ETestCase):
    """
    Select the first process in the process list and verify there is at least one entry in the list
    """

    def _execute(self, process_filter):
        # Finding FilterProcesses/ProcessList occationally throws from within pywinauto. This is not
        # understood. The while loop with the try/except block is a workaround for that.
        while True:
            try:
                filter_edit = self.find_control('Edit', 'FilterProcesses')
                break
            except KeyError:
                logging.info('Find FilterProcesses failed. Try again.')

        while True:
            try:
                process_list = self.find_control('Table', 'ProcessList')
                break
            except KeyError:
                logging.info('Find ProcessList failed. Try again.')

        logging.info('Waiting for process list to be populated')
        wait_for_condition(lambda: process_list.item_count() > 0, 30)
        logging.info('Setting filter text for process list')
        if process_filter:
            filter_edit.set_focus()
            filter_edit.set_edit_text('')
            send_keys(process_filter)
        # Wait for the process to show up - it may still be starting
        wait_for_condition(lambda: process_list.item_count() > 0, 30)

        logging.info('Process selected, continuing to main window...')
        process_list.children(control_type='DataItem')[0].double_click_input()
        _wait_for_main_window(self.suite.application)
        window = self.suite.top_window(True)
        self.expect_eq(window.class_name(), "OrbitMainWindow", 'Main window is visible')
        window.maximize()


class WaitForConnectionToTargetInstanceAndProcess(E2ETestCase):
    """
    Assumes Orbit has been started with --target_process and --target_instance parameters.
    Waits for the main window to appear and checks the contents of the target label.
    """

    def _execute(self, expected_instance_id: str, expected_process_path: str):
        _wait_for_main_window(self.suite.application, timeout=120)
        # As there is a new top window (Orbit main window), we need to update the top window.
        self.suite.top_window(True)

        # Check the target label: The full process path and instance should be in the tooltip
        stadia_target = self.find_control('Group', 'Stadia target')
        tooltip = get_tooltip(self.suite.application, stadia_target)
        self.expect_true(expected_process_path in tooltip, 'Found expected process')
        self.expect_true(expected_instance_id in tooltip, 'Found expected instance')
