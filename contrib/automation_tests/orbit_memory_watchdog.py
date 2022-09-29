"""
Copyright (c) 2022 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.capture_window import CaptureAndWaitForInterruptedWarning
from test_cases.symbols_tab import WaitForLoadingSymbolsAndCheckModuleState, FilterAndHookFunction
"""Test that OrbitService stops the capture when it is using too much memory.

Before this script is run there needs to be a gamelet reserved and
"call_foo_repeatedly" (can be downloaded from GCS) needs to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - connect to a gamelet and select a process;
 - instrument a high-frequency function with user space instrumentation;
 - start a long capture;
 - verify that the capture gets stopped by OrbitService and that the corresponding warning is shown.
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="call_foo_repeat"),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="call_foo_repeatedly"),
        FilterAndHookFunction(function_search_string="foo{(}{)}"),
        CaptureAndWaitForInterruptedWarning(user_space_instrumentation=True)
    ]
    suite = E2ETestSuite(test_name="Memory watchdog", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
