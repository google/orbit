"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import enum
import logging
import os
import time
from collections import namedtuple
from typing import List

from absl import flags

from pywinauto.keyboard import send_keys

from core.common_controls import DataViewPanel
from core.orbit_e2e import E2ETestCase, wait_for_condition, find_control

from test_cases.capture_window import Capture
from test_cases.live_tab import VerifyScopeTypeAndHitCount

Module = namedtuple("Module", ["name", "path", "state"])
CACHE_LOCATION = "{appdata}\\OrbitProfiler\\cache\\".format(appdata=os.getenv('APPDATA'))

MODULE_STATE_DISABLED = "Disabled"
MODULE_STATE_DOWNLOADING = "Downloading..."
MODULE_STATE_ERROR = "Error"
MODULE_STATE_LOADING = "Loading..."
MODULE_STATE_LOADED = "Loaded"
MODULE_STATE_PARTIAL = "Partial"

MODULE_FINAL_STATES = [
    MODULE_STATE_DISABLED, MODULE_STATE_ERROR, MODULE_STATE_LOADED, MODULE_STATE_PARTIAL
]


def _show_symbols_and_functions_tabs(top_window):
    logging.info("Showing symbols tab")
    find_control(top_window, "TabItem", "Symbols").click_input()


def _wait_for_loading_and_measure_time(top_window) -> float:
    logging.info('Waiting for all modules to be loaded...')
    all_modules_finalized = False

    start_time = time.time()
    TIMEOUT_IN_MINUTES = 10

    while not all_modules_finalized:
        if time.time() - start_time > TIMEOUT_IN_MINUTES * 60:
            raise TimeoutError("Maximum wait time for module loading exceeded")

        try:
            modules = _gather_module_states(top_window)
        # This may raise an exception if the table is updated while gathering module states
        except:
            continue
        all_modules_finalized = True
        for module in modules:
            if module.state not in MODULE_FINAL_STATES:
                all_modules_finalized = False
                break

    total_time = time.time() - start_time
    logging.info(
        "Symbol loading has completed. Total time: {time:.2f} seconds".format(time=total_time))

    return total_time


def _gather_module_states(top_window) -> List[Module]:
    result = []
    modules_dataview = DataViewPanel(find_control(top_window, "Group", "ModulesDataView"))
    for i in range(0, modules_dataview.get_row_count()):
        state = modules_dataview.get_item_at(i, 0).texts()[0]
        name = modules_dataview.get_item_at(i, 1).texts()[0]
        path = modules_dataview.get_item_at(i, 2).texts()[0]
        result.append(Module(name, path, state))

    return result


def _find_and_close_error_dialog(top_window) -> str or None:
    window = find_control(top_window,
                          'Window',
                          'Symbol Loading Error',
                          recurse=False,
                          raise_on_failure=False)
    logging.info("Trying to find Symbol Loading Error dialog")
    if window is None:
        return None

    logging.info("Trying to find module name label in the error dialog")
    # Finding the label via a find_control and an accessibleName is not possible here.
    # When adding the accessibleName to the QLabel, the content of the label is not available in
    # pywinauto anymore.
    module_name_label = window.children()[2]
    if not module_name_label:
        return None

    logging.info("Found module name label in the error dialog")
    module_path = module_name_label.texts()[0]
    if not module_path:
        return None

    logging.info("Found error dialog for module {path}, closing.".format(path=module_path))
    find_control(window, 'Button', 'Cancel').click_input()
    return module_path


ModulesLoadingResult = namedtuple("LoadingResult", ["time", "errors"])


class ClearSymbolCache(E2ETestCase):
    """
    Clear the symbol cache and verify no symbol files remain.
    """

    def _execute(self):
        logging.info("Clearing symbol cache at {location}".format(location=CACHE_LOCATION))
        for file in os.listdir(CACHE_LOCATION):
            full_path = os.path.join(CACHE_LOCATION, file)
            if os.path.isfile(full_path):
                os.unlink(full_path)
        self.expect_true(not os.listdir(CACHE_LOCATION), 'Cache is empty')


class WaitForLoadingSymbolsAndVerifyCache(E2ETestCase):
    """
    Wait for automatically loading all symbol files and checks if all of them exist in the cache. In addition, this test
    measures the total duration of symbol loading (estimated), and can fail if the duration differs from the previous
    run by a limit defined by `expected_duration_difference`.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    def _execute(self, expected_duration_difference_ratio: float = None):
        """
        :param expected_duration_difference_ratio: Expected difference as ratio relative to the previously stored value.
            If < 1.0, the current run must take *at most* expected_difference_ratio * previous_time.
            If >= 1.0, the current run must take *at least* expected_difference_ratio * previous_time.
            If None, this will only update the stored value.
            If no stored value is present from a previous run, the parameter will be treated as None, and cannot fail
            the test.
            Violating those conditions will result in test failure.
        """
        _show_symbols_and_functions_tabs(self.suite.top_window())

        loading_time = _wait_for_loading_and_measure_time(self.suite.top_window())

        self._check_and_update_duration("load_all_modules_duration", loading_time,
                                        expected_duration_difference_ratio)

        modules = _gather_module_states(self.suite.top_window())
        self._verify_at_least_one_module_is_loaded(modules)
        self._verify_all_modules_are_cached(modules)
        logging.info("Done. Loading time: {time:.2f}s, module errors: {errors}".format(
            time=loading_time,
            errors=[module.name for module in modules if module.state == MODULE_STATE_ERROR]))

    def _verify_at_least_one_module_is_loaded(self, modules: List[Module]):
        loaded_modules = [module for module in modules if module.state == MODULE_STATE_LOADED]
        self.expect_true(len(loaded_modules) != 0, "At least one loaded module.")

    def _verify_all_modules_are_cached(self, modules: List[Module]):
        loaded_modules = [module for module in modules if module.state == MODULE_STATE_LOADED]
        module_set = set(loaded_modules)
        logging.info(
            'Verifying cache. Found {total} modules in total, {loaded} of which are loaded'.format(
                total=len(modules), loaded=len(loaded_modules)))

        cached_files = [file for file in os.listdir(CACHE_LOCATION)]
        cached_file_set = set(cached_files)

        for module in loaded_modules:
            expected_filename = module.path.replace("/", "_")
            self.expect_true(expected_filename in cached_file_set,
                             'Module {expected} found in cache'.format(expected=expected_filename))
            module_set.remove(module)
            cached_file_set.remove(expected_filename)

        self.expect_eq(
            0, len(module_set),
            'All successfully loaded modules are cached. Modules not found in cache: {}'.format(
                [module.name for module in module_set]))

    def _check_and_update_duration(self, storage_key: str, current_duration: float,
                                   expected_difference_ratio: float):
        """
        Compare the current duration with the previous run, and fail if it exceeds the defined bounds.
        :param storage_key: Unique key to store the duration within the test suite, used to persist the data across
            multiple tests (uses `self.suite.shared_data`)
        :param current_duration: The duration of the current run
        :param expected_difference_ratio: @see documentation of the same parameter in _execute
        """
        last_duration = self.suite.shared_data.get(storage_key, None)
        self.suite.shared_data[storage_key] = current_duration

        if expected_difference_ratio is not None and last_duration is not None:
            expected_duration = last_duration * expected_difference_ratio
            if expected_difference_ratio < 1.0:
                self.expect_true(
                    current_duration <= expected_duration,
                    "Expected symbol loading time to be at most {expected:.2f}s."
                    "Last run duration: {last:.2f}s, current run duration: {cur:.2f}s".format(
                        expected=expected_duration, last=last_duration, cur=current_duration))
            else:
                self.expect_true(
                    current_duration >= expected_duration,
                    "Expected symbol loading time to be at least {expected:.2f}s."
                    "Last run duration: {last:.2f}s, current run duration: {cur:.2f}s".format(
                        expected=expected_duration, last=last_duration, cur=current_duration))


class WaitForLoadingSymbolsAndCheckModuleState(E2ETestCase):
    """
    Waits for automatically loading all symbol files and checks if the specified module was loaded successfully
    (or is in the desired "Symbols" state).
    """

    def _execute(self, module_search_string: str, expected_state: str = MODULE_STATE_LOADED):
        _wait_for_loading_and_measure_time(self.suite.top_window())
        VerifyModuleState(module_search_string=module_search_string,
                          expected_state=expected_state).execute(self.suite)


class WaitForLoadingSymbolsAndCheckNoErrorStates(E2ETestCase):
    """
    Waits for automatically loading all symbol files and checks that no module is in the "Error" state.
    """

    def _execute(self):
        _wait_for_loading_and_measure_time(self.suite.top_window())
        logging.info("Verifying that no module is in the 'Error' state...")
        modules = _gather_module_states(self.suite.top_window())
        self.expect_true(len(modules) > 0, "At least one module is present")
        self.expect_eq(sum(module.state == MODULE_STATE_ERROR for module in modules), 0,
                       "No module is in the 'Error' state")
        logging.info("Verified that no module is in the 'Error' state")


class LoadSymbols(E2ETestCase):
    """
    Load specified modules, wait until the UI marks them as loaded, and verifies the functions list is
    non-empty.

    Selection is done be filtering the module list and loading the first remaining row.
    """

    def _execute(self, module_search_string: str, expect_fail=False):
        """
        :param module_search_string: String to enter in the module filter field, the first entry in the list of
            remaining modules will be loaded
        :param expect_fail: If True, the test will succeed if loading symbols results in an error message
        """
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

        logging.info('Waiting for loading modules')

        # The Loading column which should get filled with the "Loaded" is the first column (0)
        if expect_fail:
            wait_for_condition(
                lambda: _find_and_close_error_dialog(self.suite.top_window()) is not None)
        else:
            wait_for_condition(
                lambda: modules_dataview.get_item_at(0, 0).texts()[0] == MODULE_STATE_LOADED, 100)

        if not expect_fail:
            VerifySymbolsLoaded(symbol_search_string=module_search_string).execute(self.suite)


class VerifyModuleState(E2ETestCase):
    """
    Verifies whether a module with the given search string is loaded (or in the desired "Symbols" state).
    """

    def _execute(self, module_search_string: str, expected_state: str = MODULE_STATE_LOADED):
        _show_symbols_and_functions_tabs(self.suite.top_window())

        logging.info('Start verifying module %s is in state %s.', module_search_string,
                     expected_state)
        modules_dataview = DataViewPanel(self.find_control("Group", "ModulesDataView"))
        wait_for_condition(lambda: modules_dataview.get_row_count() > 0, 100)
        modules_dataview.filter.set_focus()
        modules_dataview.filter.set_edit_text('')
        send_keys(module_search_string)
        wait_for_condition(lambda: modules_dataview.get_row_count() > 0)
        wait_for_condition(lambda: expected_state in modules_dataview.get_item_at(0, 0).texts()[0])


class VerifySymbolsLoaded(E2ETestCase):

    def _execute(self, symbol_search_string: str):
        logging.info('Start verifying symbols with substring %s are loaded', symbol_search_string)
        functions_dataview = DataViewPanel(self.find_control("Group", "FunctionsDataView"))

        logging.info('Filtering symbols')
        functions_dataview.filter.set_focus()
        functions_dataview.filter.set_edit_text('')
        send_keys(symbol_search_string)

        logging.info('Verifying at least one symbol with substring %s has been loaded',
                     symbol_search_string)
        wait_for_condition(lambda: functions_dataview.get_row_count() > 0)
        logging.info("Found expected symbol(s)")


selected_function_string: str = 'H'
frame_track_enabled_string: str = 'F'


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
        wait_for_condition(
            lambda: selected_function_string in functions_dataview.get_item_at(0, 0).texts()[0])


class FilterAndEnableFrameTrackForFunction(E2ETestCase):
    """
    Enable frame track for a function based on a search string, and verify it is indicated correctly in the UI.
    """

    def _execute(self, function_search_string):
        _show_symbols_and_functions_tabs(self.suite.top_window())

        logging.info('Enabling frame track for function based on search "%s"',
                     function_search_string)
        functions_dataview = DataViewPanel(self.find_control("Group", "FunctionsDataView"))

        logging.info('Waiting for function list to be populated...')
        wait_for_condition(lambda: functions_dataview.get_row_count() > 0, 100)

        logging.info('Filtering and enabling frame track')
        functions_dataview.filter.set_focus()
        functions_dataview.filter.set_edit_text('')
        send_keys(function_search_string)
        wait_for_condition(lambda: functions_dataview.get_row_count() >= 1)
        functions_dataview.get_item_at(0, 0).click_input('right')

        self.find_context_menu_item('Enable frame track(s)').click_input()

        awaited_string: str = selected_function_string + ' ' + frame_track_enabled_string
        wait_for_condition(
            lambda: awaited_string in functions_dataview.get_item_at(0, 0).texts()[0])


class UnhookAllFunctions(E2ETestCase):
    """
    Unhook all functions.
    """

    def _execute(self):
        _show_symbols_and_functions_tabs(self.suite.top_window())

        functions_dataview = DataViewPanel(self.find_control("Group", "FunctionsDataView"))
        logging.info('Waiting for function list to be populated...')
        wait_for_condition(lambda: functions_dataview.get_row_count() > 0, 100)
        functions_dataview.filter.set_focus()
        functions_dataview.filter.set_edit_text('')
        functions_dataview.get_item_at(0, 0).click_input('left')
        # Hit Ctrl+a to select all functions.
        send_keys('^a')
        functions_dataview.get_item_at(0, 0).click_input('right')
        self.find_context_menu_item('Unhook').click_input()


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
            wait_for_condition(
                lambda: selected_function_string in functions_dataview.get_item_at(i, 0).texts()[0])


class LoadAndVerifyHelloGgpPreset(E2ETestCase):
    """
    Load the predefined E2E test preset and verify if has been applied correctly.
    TODO: This can be removed when orbit_load_preset.py is removed (it is replaced by
    orbit_load_preset_2.py).
    """

    def _execute(self):
        _show_symbols_and_functions_tabs(self.suite.top_window())

        self._load_presets()
        wait_for_condition(lambda: self._try_verify_functions_are_hooked)

        Capture().execute(self.suite)

        logging.info('Verifying function call counts')
        VerifyScopeTypeAndHitCount(scope_name='DrawFrame',
                                   scope_type="D",
                                   min_hits=30,
                                   max_hits=3000).execute(self.suite)
        VerifyScopeTypeAndHitCount(scope_name='GgpIssueFrameToken',
                                   scope_type="D",
                                   min_hits=30,
                                   max_hits=3000).execute(self.suite)

    def _load_presets(self):
        presets_panel = DataViewPanel(self.find_control('Group', 'PresetsDataView'))

        draw_frame_preset_row = presets_panel.find_first_item_row('draw_frame_in_hello_ggp_1_68', 1,
                                                                  True)
        issue_frame_token_preset_row = presets_panel.find_first_item_row(
            'ggp_issue_frame_token_in_hello_ggp_1_68', 1, True)

        self.expect_true(draw_frame_preset_row is not None, 'Found draw_frame preset')
        self.expect_true(issue_frame_token_preset_row is not None,
                         'Found ggp_issue_frame_token preset')

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
        self.expect_true(
            selected_function_string in functions_panel.get_item_at(draw_frame_row, 0).texts()[0],
            'DrawFrame is marked as hooked')
        self.expect_true(
            selected_function_string in functions_panel.get_item_at(issue_frame_token_row,
                                                                    0).texts()[0],
            'GgpIssueFrameToken is marked as hooked')

        return True


class VerifyFunctionHooked(E2ETestCase):
    """
    Verfify wether or not function is hooked in the functions table in the symbols tab.
    """

    def _execute(self, function_search_string: str, expect_hooked: bool = True):
        _show_symbols_and_functions_tabs(self.suite.top_window())

        functions_dataview = DataViewPanel(self.find_control("Group", "FunctionsDataView"))
        functions_dataview.filter.set_focus()
        functions_dataview.filter.set_edit_text('')
        send_keys(function_search_string)
        wait_for_condition(
            lambda: functions_dataview.find_first_item_row(function_search_string, 1) is not None)
        row = functions_dataview.find_first_item_row(function_search_string, 1)
        if expect_hooked:
            self.expect_true(
                selected_function_string in functions_dataview.get_item_at(row, 0).texts()[0],
                'Function is marked as hooked.')
        else:
            self.expect_true(
                selected_function_string not in functions_dataview.get_item_at(row, 0).texts()[0],
                'Function is not marked as hooked.')


class PresetStatus(enum.Enum):
    LOADABLE = "loadable"
    PARTIALLY_LOADABLE = "partially loadable"
    NOT_LOADABLE = "not loadable"


class VerifyPresetStatus(E2ETestCase):
    """
    Verfify wether preset is loadable, partially loadable or not loadable.
    """

    def _execute(self, preset_name: str, expected_status: PresetStatus):
        _show_symbols_and_functions_tabs(self.suite.top_window())

        presets_panel = DataViewPanel(self.find_control('Group', 'PresetsDataView'))
        preset_row = presets_panel.find_first_item_row(preset_name, 1, True)
        self.expect_true(preset_row is not None, 'Found preset.')
        status_text = presets_panel.get_item_at(preset_row, 0).texts()[0]
        if expected_status is PresetStatus.LOADABLE:
            self.expect_true('Yes' in status_text, 'Preset is loadable.')
        if expected_status is PresetStatus.PARTIALLY_LOADABLE:
            self.expect_true('Partially' in status_text, 'Preset is partially loadable.')
        if expected_status is PresetStatus.NOT_LOADABLE:
            self.expect_true('No' in status_text, 'Preset is not loadable.')


class LoadPreset(E2ETestCase):
    """
    Load preset with given name.
    """

    def _execute(self, preset_name: str):
        _show_symbols_and_functions_tabs(self.suite.top_window())

        presets_panel = DataViewPanel(self.find_control('Group', 'PresetsDataView'))
        preset_row = presets_panel.find_first_item_row(preset_name, 1, True)
        self.expect_true(preset_row is not None, 'Found preset.')
        status_text = presets_panel.get_item_at(preset_row, 0).texts()[0]

        if 'No' in status_text:
            logging.info("About to click File->OpenPreset...")
            app_menu = self.suite.top_window().descendants(control_type="MenuBar")[1]
            app_menu.item_by_path("File->Open Preset...").click_input()

            def get_open_file_dialog():
                return self.find_control(control_type="Window",
                                         name_contains="Select a file",
                                         parent=self.suite.top_window(),
                                         recurse=False)

            logging.info("Waiting for the file open dialog to appear")
            wait_for_condition(lambda: get_open_file_dialog().is_visible())
            dialog = get_open_file_dialog()

            file_edit = self.find_control(control_type="Edit",
                                          name="File name:",
                                          parent=dialog,
                                          recurse=True)
            file_edit.type_keys(preset_name)
            file_edit.type_keys('{DOWN}{ENTER}')

            message_box = self.suite.top_window().child_window(title_re="Preset loading failed*")
            self.expect_true(message_box is not None, 'Message box found.')
            message_box.Ok.click()
        else:
            presets_panel.get_item_at(preset_row, 0).click_input(button='right')
            self.find_context_menu_item('Load Preset').click_input()
            logging.info('Loaded preset: %s', preset_name)

            if 'Partially' in status_text:
                message_box = self.suite.top_window().child_window(
                    title_re="Preset only partially loaded*")
                self.expect_true(message_box is not None, 'Message box found.')
                message_box.Ok.click()


class SavePreset(E2ETestCase):
    """
    Save current state in preset.
    """

    def _execute(self, preset_name: str):
        app_menu = self.suite.top_window().descendants(control_type="MenuBar")[1]
        app_menu.item_by_path("File->Save Preset As ...").click_input()
        dialog = self.suite.top_window().child_window(title_re="Specify*")
        dialog.FileNameCombo.type_keys(preset_name)
        dialog.Save.click()


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

        logging.info('Start showing source code for function {}'.format(function_search_string))
        functions_dataview = DataViewPanel(self.find_control("Group", "FunctionsDataView"))

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
            return self.find_control(control_type="Window",
                                     name="Source code file not found",
                                     parent=self.suite.top_window(),
                                     recurse=False)

        wait_for_condition(lambda: get_message_box().is_visible())

        logging.info('Message box found - Clicking "Choose file..."')
        choose_file_button = self.find_control(control_type="Button",
                                               name="Choose file...",
                                               parent=get_message_box(),
                                               recurse=True)
        choose_file_button.click_input()

    def _handle_file_open_dialog(self):
        logging.info("Waiting for File Open Dialog")

        def get_file_open_dialog():
            return self.find_control(control_type="Window",
                                     name_contains="Choose ",
                                     parent=self.suite.top_window(),
                                     recurse=False)

        wait_for_condition(lambda: get_file_open_dialog().is_visible())
        file_open_dialog = get_file_open_dialog()
        logging.info("File Open Dialog is now visible. Looking for file edit...")

        logging.info("File Edit was found. Entering file path...")
        file_edit = self.find_control(control_type="Edit",
                                      name="File name:",
                                      parent=file_open_dialog,
                                      recurse=True)
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
        file_open_button = self.find_control(control_type="Button",
                                             name="Open",
                                             parent=file_open_dialog,
                                             recurse=False)
        file_open_button.click_input()

    def _handle_source_code_dialog(self):
        logging.info("Waiting for the source code dialog.")

        source_code_dialog = self.suite.application.window(auto_id="CodeViewerDialog")
        source_code_dialog.wait('visible')

        logging.info("Found source code dialog. Checking contents...")
        source_code_edit = self.find_control(control_type="Edit",
                                             name="",
                                             parent=source_code_dialog,
                                             recurse=False)
        self.expect_true('#include <ggp_c/ggp.h>' in source_code_edit.get_line(0),
                         "Source code dialog shows the correct file.")

        logging.info("All good. Closing the dialog...")
        close_button = self.find_control(control_type="Button",
                                         name="Close",
                                         parent=source_code_dialog,
                                         recurse=True)
        close_button.click_input()

    def _execute(self, function_search_string: str):
        self._provoke_goto_source_action(function_search_string)
        self._handle_file_not_found_message_box()
        self._handle_file_open_dialog()
        self._handle_source_code_dialog()
