"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite, E2ETestCase
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.capture_window import Capture
from test_cases.symbols_tab import WaitForLoadingSymbolsAndCheckModuleState, FilterAndHookMultipleFunctions
from test_cases.live_tab import VerifyOneFunctionWasHit
"""Instrument function from libggp.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This test is verifying that symbols from libggp are loadable and that functions 
in the binary can be instrumented. 
This is different to "orbit_instrument_function" in such that it operates on a
binary that is shipped with the instance (not the game binary). These binaries
are potentially produced by different toolchains. 
Note that it is a bit weird to instrument something in libggp and make 
assumptions about its inner workings. Our assumption here is that
"GgpIssueFrameToken_v*" will be the name of the implementation of
"GgpIssueFrameToken". While in theory we might change the inner workings of
libggp I hope this assumption is weak enought to never become false.

This automation script covers a basic workflow:
 - connect to a gamelet
 - select a process and load debug symbols for libggp
 - instrument "GgpIssueFrameToken_v*"
 - take a capture and verify that least one of the hooked function got hit
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter='hello_'),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="libggp"),
        FilterAndHookMultipleFunctions(function_search_string='GgpIssueFrameToken_v'),
        Capture(),
        VerifyOneFunctionWasHit(function_name_contains='GgpIssueFrameToken_v',
                                min_calls=30,
                                max_calls=3000)
    ]
    suite = E2ETestSuite(test_name="Instrument libggp", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
