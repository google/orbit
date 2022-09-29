"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.capture_window import Capture, CheckTimers, VerifyTracksDoNotExist, VerifyTracksExist, ToggleCollapsedStateOfAllTracks, FilterTracks
from test_cases.symbols_tab import WaitForLoadingSymbolsAndCheckModuleState
from test_cases.live_tab import VerifyScopeTypeAndHitCount
"""Test Orbit's manual instrumentation.

Before this script is run there needs to be a gamelet reserved and
"OrbitTestManualInstrumentation 3 10 100000" has to be started. Note that the 
command line arguments matter, the test expects three worker threads.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers the following workflow:
 - connect to a gamelet
 - select a process and load debug symbols
 - take a capture without api activated, verify no manual instrumentation is reported
 - and take a capture with api activated, verify all the manual instrumentation shows up
 - take another capture without api activated, verify no manual instrumentation is reported
"""


def main(argv):
    tracks = [
        "DynamicName_0", "DynamicName_1", "DynamicName_2", "DynamicName_3", "double_var",
        "float_var", "int64_var", "int_var", "uint64_var", "uint_var", "ORBIT_ASYNC_TASKS",
        "ORBIT_START_ASYNC_TEST"
    ]
    scope_types = ["MA", "MS", "MS", "MS", "MS", "MA", "MS", "MS", "MS", "MS", "MS"]
    scope_names = [
        "ORBIT_ASYNC_TASKS", "TestFunc", "Sleep for two milliseconds",
        "Sleeping for two milliseconds with group id", "ORBIT_START_TEST with group id",
        "ORBIT_START_ASYNC_TEST", "ORBIT_START_TEST", "ORBIT_SCOPE_TEST",
        "ORBIT_SCOPE_TEST_WITH_COLOR", "BusyWork", "TestFunc2"
    ]

    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="OrbitTest"),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="OrbitTest"),
        # Take a capture without manual instrumentation and assert that no tracks show up.
        Capture(manual_instrumentation=False),
        VerifyTracksDoNotExist(track_names=tracks),
        # Take a capture with manual instrumentation and check for the existence of the tracks,
        # the timers in the thread tracks and the scopes under Live tab
        Capture(manual_instrumentation=True),
        VerifyTracksExist(track_names=tracks),
        # Occationally there are four threads named OrbitThread_*. This seems to be a side effect
        # of injecting the library for manual instrumentation. So the test below is relaxed to
        # check for three or four threads.
        FilterTracks(filter_string="OrbitThread_",
                     expected_minimum_track_count=3,
                     expected_maximum_track_count=4),
        ToggleCollapsedStateOfAllTracks(),
        # For the same reason explained above 'FilterTracks' we only check if at least one track
        # contains timers; strictly speaking there are three tracks with timers and there might
        # be anotherone without timers.
        CheckTimers(track_name_filter="OrbitThread_*", require_all=False)
    ] + [
        VerifyScopeTypeAndHitCount(scope_name=name, scope_type=type)
        for name, type in zip(scope_names, scope_types)
    ] + [
        # Take another capture without manual instrumentation and assert that the tracks are gone.
        Capture(manual_instrumentation=False),
        VerifyTracksDoNotExist(track_names=tracks),
    ]
    suite = E2ETestSuite(test_name="Manual Instrumentation", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
