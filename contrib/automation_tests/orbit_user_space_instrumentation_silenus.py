"""
Copyright (c) 2022 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.capture_window import Capture, CheckTimers
from test_cases.symbols_tab import WaitForLoadingSymbolsAndCheckModuleState, FilterAndHookFunction
from test_cases.live_tab import VerifyScopeTypeAndHitCount
"""Instrument a single function using user space instrumentation on Silenus.

Before this script is run there needs to be a gamelet reserved and our version
of "triangle.exe" with the `Render` function made `__declspec(noinline)` has to
be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - connect to a gamelet, select a process running on Silenus, load debug symbols
 - instrument a function
 - take a capture with Orbit's user space dynamic instrumentation
 - verify the hooked function is recorded
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="triangle.exe"),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="triangle.exe"),
        FilterAndHookFunction(function_search_string="Render{ }triangle.exe"),
        Capture(user_space_instrumentation=True),
        CheckTimers(track_name_filter="triangle.exe", require_all=False),
        VerifyScopeTypeAndHitCount(scope_name="Render",
                                   scope_type="D",
                                   min_hits=700,
                                   max_hits=70000),
    ]
    suite = E2ETestSuite(test_name="User Space Instrumentation on Silenus", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
