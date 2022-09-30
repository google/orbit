"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.capture_window import Capture
from test_cases.symbols_tab import WaitForLoadingSymbolsAndCheckModuleState, FilterAndHookFunction
from test_cases.live_tab import VerifyScopeTypeAndHitCount
"""Instrument a single function in Orbit using user space instrumentation.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - connect to a gamelet, select a process and load debug symbols
 - instrument a function
 - take a capture with Orbit's own dynamic instrumentation (not relying on kernel uprobes)
 - verify the hooked function is recorded
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter='hello_'),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="hello_ggp"),
        FilterAndHookFunction(function_search_string='DrawFrame'),
        Capture(user_space_instrumentation=True),
        VerifyScopeTypeAndHitCount(scope_name='DrawFrame',
                                   scope_type="D",
                                   min_hits=30,
                                   max_hits=3000),
    ]
    suite = E2ETestSuite(test_name="User Space Instrumentation", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
