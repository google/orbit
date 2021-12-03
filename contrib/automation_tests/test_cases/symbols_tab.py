"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import enum
import logging
import os
import re
import time
from collections import namedtuple
from typing import List
from abc import ABC, abstractmethod

from absl import flags

from pywinauto.keyboard import send_keys

from core.common_controls import DataViewPanel
from core.orbit_e2e import E2ETestCase, wait_for_condition, find_control

from test_cases.capture_window import Capture
from test_cases.live_tab import VerifyFunctionCallCount

Module = namedtuple("Module", ["name", "path", "is_loaded"])
CACHE_LOCATION = "{appdata}\\OrbitProfiler\\cache\\".format(appdata=os.getenv('APPDATA'))


def _show_symbols_and_functions_tabs(top_window):
    logging.info("Showing symbols tab")
    find_control(top_window, "TabItem", "Symbols").click_input()


def _find_and_close_error_dialog(top_window) -> str or None:
    window = find_control(top_window, 'Window', 'Error loading symbols', raise_on_failure=False)
    if window is None:
        return None

    error_message = find_control(window, 'Text').texts()[0]
    module_path_search_result = re.search('for module "(.+?)"', error_message)
    if not module_path_search_result:
        return None

    logging.info("Found error dialog with message {message}, closing.".format(message=error_message))
    find_control(window, 'Button').click_input()
    return module_path_search_result[1]


class DurationDiffMixin(ABC):
    """
    Provides utility methods to store and compare durations between runs, and fails the test if a certain threshold
    of duration difference is exceeded between runs.

    For example, this is used to measure symbol loading times from cold and warm caches, and verify that warm cache
    loading is significantly faster.

    Usage:
    - Inherit your E2ETestCase from this mixin
    - Use time.time() to record start times and duration
    - Where needed, call `self._check_and_update_duration` to store the current duration and fail the test if applicable
    """
    suite = None

    @abstractmethod
    def expect_true(self, cond, msg):
        pass

    @abstractmethod
    def expect_eq(self, left, right, msg):
        pass

    def _check_and_update_duration(self, storage_key: str, current_duration: float, expected_difference: float):
        """
        Compare the current duration with the previous run, and fail if it exceeds the defined bounds.
        :param storage_key: Unique key to store the duration within the test suite, used to persist the data across
            multiple tests (uses `self.suite.shared_data`)
        :param current_duration: The duration of the current run
        :param expected_difference: Expected difference in seconds. If negative, the current run must be at least
            this much *faster* than the previous run. If positive, the current run must be at least this much *slower*.
            Violating those conditions will result in test failure.
        """
        last_duration = self.suite.shared_data.get(storage_key, None)
        self.suite.shared_data[storage_key] = current_duration

        if expected_difference is not None and last_duration is not None:
            if expected_difference < 0:
                self.expect_true(current_duration <= last_duration + expected_difference,
                                 "Expected symbol loading time to be at least {diff:.2f}s faster than the last run. "
                                 "Last run duration: {last:.2f}s, current run duration: {cur:.2f}s".format(
                                     diff=-expected_difference,
                                     last=last_duration,
                                     cur=current_duration))
            else:
                self.expect_true(current_duration >= last_duration + expected_difference,
                                 "Expected symbol loading time to be at least {diff:.2f}s slower than the last run. "
                                 "Last run duration: {last:.2f}s, current run duration: {cur:.2f}s".format(
                                     diff=expected_difference,
                                     last=last_duration,
                                     cur=current_duration))


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


class LoadAllSymbolsAndVerifyCache(E2ETestCase, DurationDiffMixin):
    """
    Loads all symbol files at once and checks if all of them exist in the cache. In addition, this test measures the
    total duration of symbol loading (estimated), and can fail if the duration differs from the previous run by
    a limit defined by `expected_duration_difference`.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._modules_dataview = None

    def _execute(self, expected_duration_difference: float = None):
        """
        :param expected_duration_difference: @see DurationDiffMixin._check_and_update_duration
        """
        _show_symbols_and_functions_tabs(self.suite.top_window())
        self._modules_dataview = DataViewPanel(self.find_control("Group", "ModulesDataView"))
        self._load_all_modules()

        start_time = time.time()
        modules_loading_result = self._wait_for_loading_and_collect_errors()
        total_duration = time.time() - start_time

        self._check_and_update_duration("load_all_modules_duration", total_duration, expected_duration_difference)

        modules = self._gather_module_states()
        self._verify_all_modules_are_cached(modules)
        self._verify_all_errors_were_raised(modules, modules_loading_result.errors)
        logging.info("Done. Loading time: {time}s, module errors: {errors}".format(
            time=modules_loading_result.time, errors=modules_loading_result.errors))

    def _load_all_modules(self):
        logging.info("Loading all modules")
        self._modules_dataview.get_item_at(0, 0).click_input()
        # Select all
        send_keys("^a")
        self._modules_dataview.get_item_at(0, 0).click_input('right')
        self.find_context_menu_item('Load Symbols').click_input()

    def _verify_all_modules_are_cached(self, modules: List[Module]):
        modules = [module for module in modules if module.is_loaded]
        module_set = set(modules)
        logging.info('Verifying cache for {num} modules'.format(num=len(module_set)))

        cached_files = [file for file in os.listdir(CACHE_LOCATION)]
        cached_file_set = set(cached_files)

        for module in modules:
            expected_filename = module.path.replace("/", "_")
            self.expect_true(expected_filename in cached_file_set,
                             'Module {expected} found in cache'.format(expected=expected_filename))
            module_set.remove(module)
            cached_file_set.remove(expected_filename)

        self.expect_eq(0, len(module_set), 'All successfully loaded modules are cached')

    def _verify_all_errors_were_raised(self, all_modules: List[Module], errors: List[str]):
        modules_not_loaded = set([module.path for module in all_modules if not module.is_loaded])
        error_set = set(errors)
        for module in modules_not_loaded:
            self.expect_true(module in error_set, 'Error has been raised for module {}'.format(module))
            error_set.remove(module)
        self.expect_eq(0, len(error_set), 'All errors raised resulted in non-loadable modules')

    def _wait_for_loading_and_collect_errors(self) -> ModulesLoadingResult:
        assume_loading_complete = 0
        num_assumptions_to_be_safe = 5
        error_modules = set()
        total_time = 0

        while assume_loading_complete < num_assumptions_to_be_safe:
            start_time = time.time()

            # Since there is no "loading completed" feedback, give orbit some time to update the status message or
            # show an error dialog. Then check if any of those is visible.
            # If not, try a few more times to make sure we didn't just accidentally query the UI while the status
            # message was being updated.
            error_module = _find_and_close_error_dialog(self.suite.top_window())
            status_message = self.find_control('StatusBar').texts()[0]
            if "Copying debug info file" not in status_message and "Loading symbols" not in status_message:
                status_message = None

            if error_module is not None:
                error_modules.add(error_module)

            if not error_module and not status_message:
                assume_loading_complete += 1
            else:
                total_time += time.time() - start_time
                assume_loading_complete = 0
        logging.info("Assuming symbol loading has completed. Total time: {time:.2f} seconds".format(time=total_time))
        return ModulesLoadingResult(total_time, error_modules)

    def _gather_module_states(self) -> List[Module]:
        result = []
        for i in range(0, self._modules_dataview.get_row_count()):
            is_loaded = self._modules_dataview.get_item_at(i, 0).texts()[0] == "*"
            name = self._modules_dataview.get_item_at(i, 1).texts()[0]
            path = self._modules_dataview.get_item_at(i, 2).texts()[0]
            result.append(Module(name, path, is_loaded))

        return result


class ReplaceFileInSymbolCache(E2ETestCase):
    """
    Replace a symbol file in the cache with another file from the cache. This is used to "invalidate" symbols files
    and verify that they are downloaded again.
    """

    def _execute(self, file_to_replace, src):
        os.unlink(os.path.join(CACHE_LOCATION, file_to_replace))
        os.rename(os.path.join(CACHE_LOCATION, src), os.path.join(CACHE_LOCATION, file_to_replace))


class LoadSymbols(E2ETestCase, DurationDiffMixin):
    """
    Load specified modules, wait until the UI marks them as loaded, and verifies the functions list is
    non-empty.

    Selection is done be filtering the module list and loading the first remaining row.
    """

    def _execute(self, module_search_string: str, expected_duration_difference: float = None, expect_fail=False):
        """
        :param module_search_string: String to enter in the module filter field, the first entry in the list of
            remaining modules will be loaded
        :param expected_duration_difference: @see DurationDiffMixin._check_and_update_duration
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

        logging.info('Waiting for * to indicate loaded modules')

        # The Loading column which should get filled with the "*" is the first column (0)
        start_time = time.time()
        if expect_fail:
            wait_for_condition(lambda: _find_and_close_error_dialog(self.suite.top_window()) is not None)
        else:
            wait_for_condition(lambda: modules_dataview.get_item_at(0, 0).texts()[0] == "*", 100)
        total_time = time.time() - start_time
        self._check_and_update_duration("load_symbols_" + module_search_string, total_time,
                                        expected_duration_difference)

        VerifySymbolsLoaded(symbol_search_string=module_search_string, expect_loaded=not expect_fail).execute(
            self.suite)


class VerifyModuleLoaded(E2ETestCase):
    """
    Verifies whether a module with the give search string is loaded.
    """

    def _execute(self, module_search_string: str, expect_loaded: bool = True):
        _show_symbols_and_functions_tabs(self.suite.top_window())

        logging.info('Start verifying module %s is %s.', module_search_string,
                     "loaded" if expect_loaded else "not loaded")
        modules_dataview = DataViewPanel(self.find_control("Group", "ModulesDataView"))
        wait_for_condition(lambda: modules_dataview.get_row_count() > 0, 100)
        modules_dataview.filter.set_focus()
        modules_dataview.filter.set_edit_text('')
        send_keys(module_search_string)
        wait_for_condition(lambda: modules_dataview.get_row_count() > 0)
        self.expect_true('*' in modules_dataview.get_item_at(0, 0).texts()[0], 'Module is loaded.')


class VerifySymbolsLoaded(E2ETestCase):

    def _execute(self, symbol_search_string: str, expect_loaded: bool = True):
        logging.info('Start verifying symbols with substring %s are {}loaded'.format("" if expect_loaded else "not "),
                     symbol_search_string)
        functions_dataview = DataViewPanel(self.find_control("Group", "FunctionsDataView"))

        logging.info('Filtering symbols')
        functions_dataview.filter.set_focus()
        functions_dataview.filter.set_edit_text('')
        send_keys(symbol_search_string)
        if expect_loaded:
            logging.info('Verifying at least one symbol with substring %s has been loaded',
                         symbol_search_string)
            self.expect_true(functions_dataview.get_row_count() > 1, "Found expected symbol(s)")
        else:
            logging.info('Verifying no symbols with substring %s has been loaded',
                         symbol_search_string)
            self.expect_true(functions_dataview.get_row_count() == 0, "Found no symbols")


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
        wait_for_condition(lambda: functions_dataview.get_row_count() == 1)
        functions_dataview.get_item_at(0, 0).click_input('right')

        self.find_context_menu_item('Enable frame track(s)').click_input()
        wait_for_condition(lambda: '✓ F' in functions_dataview.get_item_at(0, 0).texts()[0])


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
            wait_for_condition(lambda: '✓' in functions_dataview.get_item_at(i, 0).texts()[0])


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
        VerifyFunctionCallCount(function_name='DrawFrame', min_calls=30,
                                max_calls=3000).execute(self.suite)
        VerifyFunctionCallCount(function_name='GgpIssueFrameToken', min_calls=30,
                                max_calls=3000).execute(self.suite)

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
        self.expect_true('✓' in functions_panel.get_item_at(draw_frame_row, 0).texts()[0],
                         'DrawFrame is marked as hooked')
        self.expect_true('✓' in functions_panel.get_item_at(issue_frame_token_row, 0).texts()[0],
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
        row = functions_dataview.find_first_item_row(function_search_string, 1)
        if expect_hooked:
            self.expect_true('✓' in functions_dataview.get_item_at(row, 0).texts()[0],
                             'Function is marked as hooked.')
        else:
            self.expect_true('✓' not in functions_dataview.get_item_at(row, 0).texts()[0],
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
            app_menu = self.suite.top_window().descendants(control_type="MenuBar")[1]
            app_menu.item_by_path("File->Open Preset...").click_input()
            dialog = self.suite.top_window().child_window(title_re="Select a file*")
            dialog.FileNameEdit.type_keys(preset_name)
            dialog.FileNameEdit.type_keys('{DOWN}{ENTER}')

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
