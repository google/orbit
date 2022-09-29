"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.capture_window import Capture
from test_cases.bottom_up_tab import VerifyHelloGgpBottomUpContents
from test_cases.symbols_tab import WaitForLoadingSymbolsAndCheckModuleState
"""Inspect the bottom-up view in Orbit using pywinauto.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - connect to a gamelet
 - select a process
 - take a capture
 - verify that the bottom-up view contains at least 10 rows
 - verify that one of the first three items is "ioctl"
 - verify that the first child of the "ioctl" item starts with "drm"
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter='hello_ggp'),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string='libc-2.24'),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string='libdrm.so.2'),
        Capture(frame_pointer_unwinding=False),
        VerifyHelloGgpBottomUpContents(),
        Capture(frame_pointer_unwinding=True, sampling_period_ms=0.1),
        VerifyHelloGgpBottomUpContents()
    ]
    suite = E2ETestSuite(test_name="Bottom-Up View", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
