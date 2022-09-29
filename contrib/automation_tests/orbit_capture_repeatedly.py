"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.capture_window import Capture, CaptureRepeatedly
from test_cases.symbols_tab import WaitForLoadingSymbolsAndCheckModuleState, FilterAndEnableFrameTrackForFunction
"""Repeatedly take very short captures using pywinauto.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers the following workflow:
 - connect to a gamelet
 - select a process
 - load symbols for some modules
 - instrument and enable frame tracks for two functions
 - repeatedly start and stop captures using F5 and verify that Orbit does not crash
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter='hello_'),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="hello_ggp"),
        FilterAndEnableFrameTrackForFunction(function_search_string='DrawFrame'),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="libvulkan.so.1"),
        FilterAndEnableFrameTrackForFunction(function_search_string='vkQueuePresentKHR'),
        Capture(length_in_seconds=1),
        CaptureRepeatedly(number_of_f5_presses=200)
    ]
    suite = E2ETestSuite(test_name="Capture repeatedly", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
