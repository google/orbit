"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging
import os

from absl import flags

from pywinauto.keyboard import send_keys

from core.common_controls import DataViewPanel
from core.orbit_e2e import E2ETestCase, wait_for_condition, find_control

from test_cases.capture_window import Capture
from test_cases.live_tab import VerifyFunctionCallCount


def _show_symbols_and_functions_tabs(top_window):
    logging.info("Showing symbols tab")
    find_control(top_window, "TabItem", "Symbols").click_input()


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
        modules_dataview.filter.set_focus()
        modules_dataview.filter.set_edit_text('')
        send_keys(module_search_string)
        wait_for_condition(lambda: modules_dataview.get_row_count() == 1)
        modules_dataview.get_item_at(0, 0).click_input('right')

        self.find_context_menu_item('Load Symbols').click_input()

        logging.info('Waiting for * to indicate loaded modules')

        wait_for_condition(lambda: modules_dataview.get_item_at(0, 4).texts()[0] == "*", 100)

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
        functions_dataview.filter.set_focus()
        functions_dataview.filter.set_edit_text('')
        send_keys(function_search_string)
        wait_for_condition(lambda: functions_dataview.get_row_count() == 1)
        functions_dataview.get_item_at(0, 0).click_input('right')

        self.find_context_menu_item('Hook').click_input()
        wait_for_condition(lambda: '✓' in functions_dataview.get_item_at(0, 0).texts()[0])
        
class FilterAndHookMultipleFunctions(E2ETestCase):
    """
    Hook multiple functions based on a search string, and verify it is indicated as hooked in the UI.
    """
    def _execute(self, function_search_string):
        _show_symbols_and_functions_tabs(self.suite.top_window())

        logging.info('Hooking functions based on search "%s"', function_search_string)
        functions_dataview = DataViewPanel(self.find_control("Group", "FunctionsDataView"))

        logging.info('Waiting for function list to be populated...')
        wait_for_condition(lambda: functions_dataview.get_row_count() > 0, 100)

        logging.info('Filtering and hooking')
        functions_dataview.filter.set_focus()
        functions_dataview.filter.set_edit_text('')
        send_keys(function_search_string)
        wait_for_condition(lambda: functions_dataview.get_row_count() > 0)
        
        for i in range(functions_dataview.get_row_count()):
            functions_dataview.get_item_at(i, 0).click_input('right')
            self.find_context_menu_item('Hook').click_input()
            wait_for_condition(lambda: '✓' in functions_dataview.get_item_at(i, 0).texts()[0])

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


class ShowSourceCode(E2ETestCase):
    """
    Select given function in Symbols Tab, right-click to open context menu, select "Go To Source"
    It is expected that the source code file is not found locally. A message box will pop up.
    The test will click "Choose file..." which brings up a file open dialog. The test types in
    the path to hello_ggp's main.c source code file. This path is hard-coded to match the
    path which is used in the testrunner workflow, but can be overwritten by a command line flag
    "--source_code_file".

    Selection is done be filtering the function list and loading the first remaining row.
    """

    def _provoke_goto_source_action(self, function_search_string: str):
        _show_symbols_and_functions_tabs(self.suite.top_window())

        logging.info('Start showing source code for function {}'.format(
            function_search_string))
        functions_dataview = DataViewPanel(
            self.find_control("Group", "FunctionsDataView"))

        logging.info('Waiting for function list to be populated...')
        wait_for_condition(lambda: functions_dataview.get_row_count() > 0, 100)

        logging.info('Filtering functions')
        functions_dataview.filter.set_focus()
        functions_dataview.filter.set_edit_text('')
        send_keys(function_search_string)
        wait_for_condition(lambda: functions_dataview.get_row_count() == 1)
        functions_dataview.get_item_at(0, 0).click_input('right')

        logging.info('Click on "Go to Source code"')
        self.find_context_menu_item('Go to Source code').click_input()

    def _handle_file_not_found_message_box(self):
        logging.info('Waiting for message box')

        def get_message_box():
            return self.find_control(control_type="Window", name="Source code file not found", parent=self.suite.top_window(), recurse=False)
        wait_for_condition(lambda: get_message_box().is_visible())

        logging.info('Message box found - Clicking "Choose file..."')
        choose_file_button = self.find_control(
            control_type="Button", name="Choose file...", parent=get_message_box(), recurse=True)
        choose_file_button.click_input()

    def _handle_file_open_dialog(self):
        logging.info("Waiting for File Open Dialog")

        def get_file_open_dialog():
            return self.find_control(control_type="Window", name_contains="Choose ", parent=self.suite.top_window(), recurse=False)
        wait_for_condition(lambda: get_file_open_dialog().is_visible())
        file_open_dialog = get_file_open_dialog()
        logging.info(
            "File Open Dialog is now visible. Looking for file edit...")

        logging.info("File Edit was found. Entering file path...")
        file_edit = self.find_control(
            control_type="Edit", name="File name:", parent=file_open_dialog, recurse=True)
        file_edit.set_focus()
        file_edit.set_edit_text('')

        # The test needs the source code file "main.c" from the hello_ggp example
        # to be available on the machine. On the E2E test infrastructure the example
        # project will be extracted to C:\build\scratch\test\hello_ggp.
        #
        # To run this test in developer machines, the patch can be overwritten by a
        # command line flag. Keep in mind, it needs to be an absolute path, which
        # is understood by the Windows file open dialog, when pasted into the filename
        # field.
        send_keys(flags.FLAGS.source_code_file)

        logging.info("Clicking the open button...")
        file_open_button = self.find_control(
            control_type="Button", name="Open", parent=file_open_dialog, recurse=False)
        file_open_button.click_input()

    def _handle_source_code_dialog(self):
        logging.info("Waiting for the source code dialog.")

        def get_source_code_dialog():
            return self.find_control(control_type="Window", name="main.c", parent=self.suite.top_window(), recurse=False)
        wait_for_condition(lambda: get_source_code_dialog().is_visible())
        source_code_dialog = get_source_code_dialog()

        logging.info("Found source code dialog. Checking contents...")
        source_code_edit = self.find_control(
            control_type="Edit", name="", parent=source_code_dialog, recurse=False)
        self.expect_true('#include <ggp_c/ggp.h>' in source_code_edit.get_line(0),
                         "Source code dialog shows the correct file.")

        logging.info("All good. Closing the dialog...")
        close_button = self.find_control(
            control_type="Button", name="Close", parent=source_code_dialog, recurse=True)
        close_button.click_input()

    def _execute(self, function_search_string: str):
        self._provoke_goto_source_action(function_search_string)
        self._handle_file_not_found_message_box()
        self._handle_file_open_dialog()
        self._handle_source_code_dialog()
