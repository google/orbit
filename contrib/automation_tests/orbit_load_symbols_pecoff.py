"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.symbols_tab import WaitForLoadingSymbolsAndCheckModuleState
from test_cases.symbols_tab import VerifySymbolsLoaded
"""Load symbols for PE/COFF modules.

Before this script is run there needs to be a gamelet reserved and
our "triangle application" has to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - connect to a gamelet
 - select a process and load debug symbols for a binary and a shared object
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter='triangle'),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="triangle.exe"),
        VerifySymbolsLoaded(symbol_search_string="wWinMain"),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="d3d11.dll"),
        VerifySymbolsLoaded(symbol_search_string="Present")
    ]
    suite = E2ETestSuite(test_name="Load Symbols PE/COFF", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
