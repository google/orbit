"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.capture_window import Capture
from test_cases.top_down_tab import VerifyHelloGgpTopDownContents
from test_cases.symbols_tab import WaitForLoadingSymbolsAndCheckModuleState
"""Inspect the top-down view in Orbit using pywinauto.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - connect to a gamelet
 - select a process
 - take a capture
 - verify that the top-down view contains at least 3 rows
 - verify that the first item is "hello_* (all threads)"
 - verify that the second item is "hello_*]" or "Ggp*]"
 - verify that the children of the first item are "*clone" and "_start" (in any order)
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter='hello_ggp'),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="libc-2.24"),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="hello_ggp"),
        Capture(frame_pointer_unwinding=False),
        VerifyHelloGgpTopDownContents(),
        Capture(frame_pointer_unwinding=True),
        VerifyHelloGgpTopDownContents()
    ]
    suite = E2ETestSuite(test_name="Top-Down View", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
