"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging
import time

from pywinauto.application import Application
from pywinauto.keyboard import send_keys

from core.orbit_e2e import E2ETestCase, wait_for_condition


def wait_for_main_window(application: Application):
    wait_for_condition(lambda: application.top_window().class_name() == "OrbitMainWindow", 30)


def get_number_of_instances_in_list(test_case: E2ETestCase) -> int:
    instance_list = test_case.find_control('Table', 'InstanceList')
    instance_count = instance_list.item_count()
    logging.info('Found %s rows in the instance list', instance_count)
    return instance_count


class ConnectToStadiaInstance(E2ETestCase):
    """
    Connect to the first available stadia instance
    """

    def _execute(self):
        window = self.suite.top_window()

        logging.info('Start connecting to gamelet.')
        connect_radio = self.find_control('RadioButton', 'ConnectToStadia')
        connect_radio.click_input()

        # Wait for the first data item in the instance list to exist
        # We're not using find_control here because magic lookup enables us to easily wait for the existence of a row
        window.InstanceList.click_input()
        window.InstanceList.DataItem0.wait('exists', timeout=100)
        self.expect_true(get_number_of_instances_in_list(self) >= 1, 'Found at least one instance')

        window.InstanceList.DataItem0.double_click_input()
        logging.info('Connecting to Instance, waiting for the process list...')

        # In the new UI, use small waits until the process list is active, and then some more for the
        # semi-transparent "loading" Overlay of the tables to disappear
        wait_for_condition(
            lambda: self.find_control('Custom', 'ProcessesFrame').is_enabled() is True, 25)
        wait_for_condition(lambda: self.find_control('Table', 'ProcessList').is_active(), 10)
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
        self.expect_true(get_number_of_instances_in_list(self) >= 1, 'Found at least one instance')
        wait_for_condition(lambda: self.find_control(
            'Group', 'ProcessListOverlay', raise_on_failure=False) is None)


class RefreshStadiaInstanceList(E2ETestCase):
    """
    Click the refresh button and check that the loading overlay is displayed and the list contains
    items when the loading finished.
    """

    def _execute(self):
        instance_list = self.find_control('Table', 'InstanceList')
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
        self.expect_true(get_number_of_instances_in_list(self) >= 1, 'Found at least one instance')


class FilterAndSelectFirstProcess(E2ETestCase):
    """
    Select the first process in the process list and verify there is at least one entry in the list
    """

    def _execute(self, process_filter):
        # Finding FilterProcesses/ProcessList occationally throws from within pywinauto. This is not
        # understood. The while loop with the try/except block is a workaround for that.
        while (True):
            try:
                filter_edit = self.find_control('Edit', 'FilterProcesses')
                break
            except KeyError:
                logging.info('Find FilterProcesses failed. Try again.')

        while (True):
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
        wait_for_main_window(self.suite.application)
        window = self.suite.top_window(True)
        self.expect_eq(window.class_name(), "OrbitMainWindow", 'Main window is visible')
        window.maximize()
