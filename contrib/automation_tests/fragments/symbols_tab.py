"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging

from absl import flags

from core.common_controls import DataViewPanel
from core.orbit_e2e import E2ETestCase, wait_for_condition, find_control


def _show_symbols_and_functions_tabs(top_window):
    logging.info("Showing symbols tab")
    if flags.FLAGS.enable_ui_beta:
        find_control(top_window, "TabItem", "Symbols").click_input()
    else:
        find_control(top_window, "TabItem", "Home").click_input()
        find_control(top_window,"TabItem", "Functions").click_input()


class LoadSymbols(E2ETestCase):
    """
    Load specified modules, wait until the UI marks them as loaded, and verifies the functions list is
    non-empty.

    Selection is done be filtering the module list and loading the first remaining row.
    """
    def _execute(self, module_search_string: str):
        _show_symbols_and_functions_tabs(self.suite.top_window())

        logging.info('Start loading symbols for module %s', module_search_string)
        modules_dataview = DataViewPanel(self.find_control("Group", "DataViewPanelModules"))

        logging.info('Waiting for module list to be populated...')
        wait_for_condition(lambda: modules_dataview.get_row_count() > 0, 100)

        logging.info('Filtering and loading')
        modules_dataview.filter.set_edit_text(module_search_string)
        wait_for_condition(lambda: modules_dataview.get_row_count() == 1)
        modules_dataview.get_item_at(0, 0).click_input('right')

        self.find_context_menu_item('Load Symbols').click_input()

        logging.info('Waiting for * to indicate loaded modules')

        wait_for_condition(lambda: modules_dataview.get_item_at(0, 4).texts()[0] == "*")

        functions_dataview = DataViewPanel(self.find_control("Group", "DataViewPanelFunctions"))
        wait_for_condition(lambda: functions_dataview.get_row_count() > 0)


class FilterAndHookFunction(E2ETestCase):
    """
    Hook a function based on a search string, and verify it is indicated as Hooked in the UI.
    """
    def _execute(self, function_search_string):
        _show_symbols_and_functions_tabs(self.suite.top_window())

        logging.info('Hooking function based on search "%s"', function_search_string)
        functions_dataview = DataViewPanel(self.find_control("Group", "DataViewPanelFunctions"))

        logging.info('Waiting for function list to be populated...')
        wait_for_condition(lambda: functions_dataview.get_row_count() > 0, 100)

        logging.info('Filtering and hooking')
        functions_dataview.filter.set_edit_text(function_search_string)
        wait_for_condition(lambda: functions_dataview.get_row_count() == 1)
        functions_dataview.get_item_at(0, 0).click_input('right')

        self.find_context_menu_item('Hook').click_input()
        wait_for_condition(lambda: '✓' in functions_dataview.get_item_at(0, 0).texts()[0])
