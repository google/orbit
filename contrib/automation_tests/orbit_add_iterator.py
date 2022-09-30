"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""
from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.symbols_tab import WaitForLoadingSymbolsAndCheckModuleState, FilterAndHookFunction
from test_cases.capture_window import Capture
from test_cases.live_tab import AddIterator
"""Add a function iterator in Orbit using pywinauto.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - connect to a gamelet
 - select a process and load debug symbols
 - instrument a function
 - take a capture
 - in the "Live" tab, add an iterator for the instrumented function
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="hello_ggp"),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="hello_ggp"),
        FilterAndHookFunction(function_search_string='DrawFrame'),
        Capture(),
        AddIterator(function_name="DrawFrame")
    ]
    suite = E2ETestSuite(test_name="Add Iterator", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
