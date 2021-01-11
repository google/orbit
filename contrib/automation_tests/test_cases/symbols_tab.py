"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging

from absl import flags

from core.common_controls import DataViewPanel
from core.orbit_e2e import E2ETestCase, wait_for_condition, find_control

from test_cases.capture_window import Capture
from test_cases.live_tab import VerifyFunctionCallCount


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
        modules_dataview = DataViewPanel(self.find_control("Group", "ModulesDataView"))

        logging.info('Waiting for module list to be populated...')
        wait_for_condition(lambda: modules_dataview.get_row_count() > 0, 100)

        logging.info('Filtering and loading')
        modules_dataview.filter.set_edit_text(module_search_string)
        wait_for_condition(lambda: modules_dataview.get_row_count() == 1)
        modules_dataview.get_item_at(0, 0).click_input('right')

        self.find_context_menu_item('Load Symbols').click_input()

        logging.info('Waiting for * to indicate loaded modules')

        wait_for_condition(lambda: modules_dataview.get_item_at(0, 4).texts()[0] == "*")

        functions_dataview = DataViewPanel(self.find_control("Group", "FunctionsDataView"))
        wait_for_condition(lambda: functions_dataview.get_row_count() > 0)


class FilterAndHookFunction(E2ETestCase):
    """
    Hook a function based on a search string, and verify it is indicated as Hooked in the UI.
    """
    def _execute(self, function_search_string):
        _show_symbols_and_functions_tabs(self.suite.top_window())

        logging.info('Hooking function based on search "%s"', function_search_string)
        functions_dataview = DataViewPanel(self.find_control("Group", "FunctionsDataView"))

        logging.info('Waiting for function list to be populated...')
        wait_for_condition(lambda: functions_dataview.get_row_count() > 0, 100)

        logging.info('Filtering and hooking')
        functions_dataview.filter.set_edit_text(function_search_string)
        wait_for_condition(lambda: functions_dataview.get_row_count() == 1)
        functions_dataview.get_item_at(0, 0).click_input('right')

        self.find_context_menu_item('Hook').click_input()
        wait_for_condition(lambda: '✓' in functions_dataview.get_item_at(0, 0).texts()[0])


class LoadAndVerifyHelloGgpPreset(E2ETestCase):
    """
    Load the predefined E2E test preset and verify if has been applied correctly.
    TODO: This may need to be updated
    """
    def _execute(self):
        _show_symbols_and_functions_tabs(self.suite.top_window())

        self._load_presets()
        wait_for_condition(lambda: self._try_verify_functions_are_hooked)

        Capture().execute(self.suite)

        logging.info('Verifying function call counts')
        VerifyFunctionCallCount(function_name='DrawFrame', min_calls=30, max_calls=3000).execute(self.suite)
        VerifyFunctionCallCount(function_name='GgpIssueFrameToken', min_calls=30, max_calls=3000).execute(self.suite)

    def _load_presets(self):
        presets_panel = DataViewPanel(self.find_control('Group', 'PresetsDataView'))

        draw_frame_preset_row = presets_panel.find_first_item_row('draw_frame_in_hello_ggp_1_52', 1, True)
        issue_frame_token_preset_row = presets_panel.find_first_item_row(
            'ggp_issue_frame_token_in_hello_ggp_1_52', 1, True)

        self.expect_true(draw_frame_preset_row is not None, 'Found draw_frame preset')
        self.expect_true(issue_frame_token_preset_row is not None, 'Found ggp_issue_frame_token preset')

        presets_panel.get_item_at(draw_frame_preset_row, 0).click_input(button='right')
        self.find_context_menu_item('Load Preset').click_input()
        logging.info('Loaded Preset DrawFrame')

        presets_panel.get_item_at(issue_frame_token_preset_row, 0).click_input(button='right')
        self.find_context_menu_item('Load Preset').click_input()
        logging.info('Loaded Preset GgpIssueFrameToken')

    def _try_verify_functions_are_hooked(self):
        logging.info('Finding rows in the function list')
        functions_panel = DataViewPanel(self.find_control('Group', 'FunctionsDataViewD'))
        draw_frame_row = functions_panel.find_first_item_row('DrawFrame', 1)
        issue_frame_token_row = functions_panel.find_first_item_row('GgpIssueFrameToken', 1)

        if draw_frame_row is None:
            return False
        if issue_frame_token_row is None:
            return False

        logging.info('Verifying hook status of functions')
        self.expect_true('✓' in functions_panel.get_item_at(draw_frame_row, 0).texts()[0],
                         'DrawFrame is marked as hooked')
        self.expect_true('✓' in functions_panel.get_item_at(issue_frame_token_row, 0).texts()[0],
                         'GgpIssueFrameToken is marked as hooked')

        return True
