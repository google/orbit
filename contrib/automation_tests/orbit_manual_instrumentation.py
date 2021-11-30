"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.capture_window import Capture, CheckTimers, VerifyTracksDoNotExist, VerifyTracksExist, ToggleCollapsedStateOfAllTracks, FilterTracks
from test_cases.symbols_tab import LoadSymbols
"""Test Orbit's manual instrumentation.

Before this script is run there needs to be a gamelet reserved and
"OrbitTestManualInstrumentation 3 10 100000" has to be started. Note that the 
command line arguments matter, the test expects three worker threads.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test it needs
to by run from 64 bit python.

This automation script covers the following workflow:
 - start Orbit
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
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter='OrbitTest'),
        LoadSymbols(module_search_string="OrbitTest"),
        # Take a capture without manual instrumentation and assert that no tracks show up.
        Capture(manual_instrumentation=False),
        VerifyTracksDoNotExist(track_names=tracks),
        # Take a capture with manual instrumentation and check for the existence of the tracks and the timers in the thread tracks.
        Capture(manual_instrumentation=True),
        VerifyTracksExist(track_names=tracks),
        FilterTracks(filter_string="OrbitThread_", expected_track_count=3),
        ToggleCollapsedStateOfAllTracks(),
        CheckTimers(track_name_filter="OrbitThread_*"),
        # Take another capture without manual instrumentation and assert that the tracks are gone.
        Capture(manual_instrumentation=False),
        VerifyTracksDoNotExist(track_names=tracks),
    ]
    suite = E2ETestSuite(test_name="Instrument Function", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
