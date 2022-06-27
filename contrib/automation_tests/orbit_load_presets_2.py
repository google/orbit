"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.symbols_tab import FilterAndHookFunction, SavePreset, UnhookAllFunctions, LoadPreset, \
    VerifyFunctionHooked, VerifyPresetStatus, PresetStatus, WaitForLoadingSymbolsAndVerifyCache
from test_cases.capture_window import Capture
from test_cases.live_tab import VerifyScopeTypeAndHitCount
from test_cases.main_window import EndSession
"""Verify preset functionality in Orbit works as expected.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started. Four presets named:

  draw_frame_in_hello_ggp_1_68.opr
  ggp_issue_frame_token_in_hello_ggp_1_68.opr
  partially_loadable.opr
  not_loadable.opr

need to exist in the preset folder. The first two are hooking the functions
indicated by their name. These are created with Orbit 1.68 so this is testing
compatibility as well. The two others are partially loadable and not loadable
when hello_ggp is profiled. Details are in the inline comments below.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from  64 bit python.
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="hello_ggp"),
        WaitForLoadingSymbolsAndVerifyCache(),
        # Load presets and verify the respective functions get hooked.
        LoadPreset(preset_name='draw_frame_in_hello_ggp_1_68'),
        VerifyFunctionHooked(function_search_string='DrawFrame'),
        LoadPreset(preset_name='ggp_issue_frame_token_in_hello_ggp_1_68'),
        VerifyFunctionHooked(function_search_string='GgpIssueFrameToken'),
        # Verify that the functions from the presets end up in the capture.
        Capture(),
        VerifyScopeTypeAndHitCount(scope_name='DrawFrame',
                                   scope_type="D",
                                   min_hits=30,
                                   max_hits=3000),
        VerifyScopeTypeAndHitCount(scope_name='GgpIssueFrameToken',
                                   scope_type="D",
                                   min_hits=30,
                                   max_hits=3000),
        # Create a new preset. Unhooking and applying this preset results in the function being hooked.
        UnhookAllFunctions(),
        FilterAndHookFunction(function_search_string="ClockNowMicroSeconds"),
        SavePreset(preset_name="clock_now_micro_seconds_in_hello_ggp.opr"),
        UnhookAllFunctions(),
        LoadPreset(preset_name='clock_now_micro_seconds_in_hello_ggp'),
        VerifyFunctionHooked(function_search_string='ClockNowMicroSeconds'),
        # Test the status of different presets.
        UnhookAllFunctions(),
        VerifyPresetStatus(preset_name='clock_now_micro_seconds_in_hello_ggp',
                           expected_status=PresetStatus.LOADABLE),
        VerifyPresetStatus(preset_name='partially_loadable',
                           expected_status=PresetStatus.PARTIALLY_LOADABLE),
        VerifyPresetStatus(preset_name='not_loadable', expected_status=PresetStatus.NOT_LOADABLE),
        # Test proper warnings when applying partially loadable or not loadable presets.
        LoadPreset(preset_name='partially_loadable'),
        VerifyFunctionHooked(function_search_string='__GI___clock_gettime'),
        UnhookAllFunctions(),
        LoadPreset(preset_name='not_loadable'),
        # Test for proper preset status when switching processes.
        EndSession(),
        FilterAndSelectFirstProcess(process_filter="OrbitService"),
        VerifyPresetStatus(preset_name='clock_now_micro_seconds_in_hello_ggp',
                           expected_status=PresetStatus.NOT_LOADABLE),
        VerifyPresetStatus(preset_name='partially_loadable',
                           expected_status=PresetStatus.PARTIALLY_LOADABLE),
        VerifyPresetStatus(preset_name='not_loadable', expected_status=PresetStatus.NOT_LOADABLE),
        # Load a partially loadable preset
        LoadPreset(preset_name='partially_loadable'),
        VerifyFunctionHooked(function_search_string='__GI___clock_gettime'),
    ]
    suite = E2ETestSuite(test_name="Load Preset", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
