"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging

from core.orbit_e2e import E2ETestCase, find_control, OrbitE2EError, wait_for_condition
from pywinauto.keyboard import send_keys


def _show_and_get_symbol_location_ui(top_window):
    control = find_control(top_window, "Window", "Symbol Locations", raise_on_failure=False)
    # Early-out: Symbol locations window is already open
    if control is not None:
        return control

    # ... if we're still here, open the symbol locations window from the main menu
    find_control(top_window, "MenuItem", "Settings").click_input()
    find_control(top_window, "MenuItem", "Symbol Locations...").click_input()
    return find_control(top_window, "Window", "Symbol Locations")


def _verify_symbols_list_contains(symbol_path_list, location):
    logging.info("Verifying symbol path location is in the list")
    location = location.replace("\\", "/")
    for item in symbol_path_list.descendants(control_type='ListItem'):
        if item.texts()[0] == location:
            return

    raise OrbitE2EError("Found symbol file location {} in the list".format(location))

def _wait_for_symbol_location_ui_close(top_window):
    wait_for_condition(
        lambda: find_control(top_window, "Window", "Symbol Locations", raise_on_failure=False) is None,
        max_seconds=30)


class AddSymbolLocation(E2ETestCase):
    """
    Add a single path to the symbol locations UI, and verify it shows up in the list of symbol locations.

    If the symbol locations window is not open, it will be opened automatically. Regardless of the previous
    state, that window will be closed after the test is done.
    """

    def _execute(self, location):
        ui = _show_and_get_symbol_location_ui(self.suite.top_window())
        self.find_control('Button', 'Add Folder', ui).click_input()
        wait_for_condition(lambda: self.find_control('Edit', 'Folder:', parent=ui) is not None,
                           max_seconds=180)
        self.find_control('Edit', 'Folder:', parent=ui).set_text(location)
        self.find_control('Button', 'Select Folder', parent=ui).click_input()
        symbol_path_list = self.find_control('List', parent=ui)
        _verify_symbols_list_contains(symbol_path_list, location)
        self.find_control('Button', 'Done', parent=ui).click_input()
        _wait_for_symbol_location_ui_close(self.suite.top_window())


class AddSymbolFile(E2ETestCase):
    """
    Add a single file to the symbol locations UI, and verify it shows up in the list of symbol locations.

    If the symbol locations window is not open, it will be opened automatically. Regardless of the previous
    state, that window will be closed after the test is done.
    """

    def _execute(self, location):
        ui = _show_and_get_symbol_location_ui(self.suite.top_window())
        self.find_control('Button', 'Add File', ui).click_input()
        wait_for_condition(lambda: self.find_control('Edit', 'File name:', parent=ui) is not None,
                           max_seconds=180)
        self.find_control('Edit', 'File name:', parent=ui).set_text(location)
        # Windows decides to show a drop down menu here with suggestions which file to pick. This is not needed
        # here, but the result is that a click on the open button is not registered. To circumvent that, a return
        # key press is done instead.
        send_keys('{VK_RETURN}')
        symbol_path_list = self.find_control('List', parent=ui)
        _verify_symbols_list_contains(symbol_path_list, location)
        self.find_control('Button', 'Done', parent=ui).click_input()
        _wait_for_symbol_location_ui_close(self.suite.top_window())


class ClearAllSymbolLocations(E2ETestCase):
    """
    Delete all symbol locations from the symbol locations window, and verify the list is empty afterwards.

    If the symbol locations window is not open, it will be opened automatically. Regardless of the previous
    state, that window will be closed after the test is done.
    """

    def _execute(self):
        ui = _show_and_get_symbol_location_ui(self.suite.top_window())
        symbol_path_list = self.find_control('List', parent=ui)
        while len(symbol_path_list.descendants(control_type='ListItem')) > 0:
            symbol_path_list.descendants(control_type='ListItem')[0].click_input()
            self.find_control('Button', 'Remove').click_input()
        self.expect_eq(0, len(symbol_path_list.descendants(control_type='ListItem')),
                       'List is empty')
        self.find_control('Button', 'Done', parent=ui).click_input()
        _wait_for_symbol_location_ui_close(self.suite.top_window())


class ToggleEnableStadiaSymbolStore(E2ETestCase):
    """
    Set the "Enable Stadia symbol store" option. 

    If the symbol locations window is not open, it will be opened automatically. Regardless of the previous
    state, that window will be closed after the test is done.
    """

    def _execute(self, enable_stadia_symbol_store: bool = True):
        ui = _show_and_get_symbol_location_ui(self.suite.top_window())
        checkbox = self.find_control('CheckBox', 'EnableStadiaSymbolStoreCheckBox', parent=ui)
        if checkbox.get_toggle_state() != enable_stadia_symbol_store:
            logging.info('Toggling "Enable Stadia symbol store" checkbox to {}.'.format(enable_stadia_symbol_store))
            checkbox.click_input()
        self.find_control('Button', 'Done', parent=ui).click_input()
        _wait_for_symbol_location_ui_close(self.suite.top_window())
