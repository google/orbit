"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.capture_window import Capture, CheckTimers, FilterTracks, VerifyTracksExist
from test_cases.symbols_tab import WaitForLoadingSymbolsAndCheckModuleState
"""Test Orbit's manual instrumentation on Silenus.

Before this script is run there needs to be a gamelet reserved and the
version of "triangle.exe" with manual instrumentation has to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers the following workflow:
 - connect to a gamelet
 - select a process running on Wine
 - load debug symbols
 - take a capture with API activated, verify all the manual instrumentation shows up
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="triangle.exe"),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="triangle.exe"),
        Capture(manual_instrumentation=True),
        VerifyTracksExist(track_names=[
            "Frame time, ms (double)", "Frame time, ms (float)", "Frame time, us (int)",
            "Frame time, us (int64)", "Frame time, us (uint)", "Frame time, us (uint64)",
            "Render (async)"
        ]),
        VerifyTracksExist(track_names="triangle.exe", allow_duplicates=True),
        FilterTracks(filter_string="Render"),
        CheckTimers(track_name_filter="Render (async)"),
        FilterTracks(filter_string="triangle.exe"),
        # There can be multiple tracks with the name of the process, and we can't rename threads on Windows,
        # hence `require_all=False`.
        CheckTimers(track_name_filter="triangle.exe", require_all=False),
    ]
    suite = E2ETestSuite(test_name="Manual Instrumentation on Silenus", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
