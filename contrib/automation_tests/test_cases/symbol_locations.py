"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging

from core.orbit_e2e import E2ETestCase, find_control, OrbitE2EError


def _show_and_get_symbol_location_ui(top_window):
    control = find_control(top_window, "Window", "Symbol Locations", raise_on_failure=False)
    # Early-out: Symbol locations window is already open
    if control is not None:
        return control

    # ... if we're still here, open the symbol locations window from the main menu
    find_control(top_window, "MenuItem", "Settings").click_input()
    find_control(top_window, "MenuItem", "Symbol Locations...").click_input()
    return find_control(top_window, "Window", "Symbol Locations")


class AddSymbolLocation(E2ETestCase):
    """
    Add a single path to the symbol locations UI, and verify it shows up in the list of symbol locations.

    If the symbol locations window is not open, it will be opened automatically. Regardless of the previous
    state, that window will be closed after the test is done.
    """

    def _execute(self, location):
        ui = _show_and_get_symbol_location_ui(self.suite.top_window())
        self.find_control('Button', 'Add Folder', ui).click_input()
        self.find_control('Edit', 'Folder:', parent=ui).set_text(location)
        self.find_control('Button', 'Select Folder', parent=ui).click_input()
        self._verify_symbols_list_contains(ui, location)
        self.find_control('Button', 'OK', parent=ui).click_input()

    def _verify_symbols_list_contains(self, ui, location):
        logging.info("Verifying symbol path location is in the list")
        location = location.replace("\\", "/")
        symbol_path_list = self.find_control('List', parent=ui)
        for item in symbol_path_list.descendants(control_type='ListItem'):
            if item.texts()[0] == location:
                return

        raise OrbitE2EError("Found symbol file location {} in the list".format(location))


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
        self.expect_eq(0, len(symbol_path_list.descendants(control_type='ListItem')), 'List is empty')
        self.find_control('Button', 'OK', parent=ui).click_input()
