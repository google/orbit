"""
Copyright (c) 2022 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.bottom_up_tab import VerifyBottomUpContentForTriangleExe
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.capture_window import Capture
from test_cases.sampling_tab import VerifySamplingContentForTriangleExe
from test_cases.symbols_tab import WaitForLoadingSymbolsAndCheckModuleState
from test_cases.top_down_tab import VerifyTopDownContentForTriangleExe
"""Verify mixed (DWARF and PE/COFF) callstack unwinding by inspecting the "Sampling", "Top-Down", "Bottom-Up" tabs.

Before this script is run there needs to be a gamelet reserved and our "triangle.exe" has to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - connect to a gamelet;
 - select a process;
 - load symbols;
 - take a capture;
 - verify the top functions by inclusive time in the "Sampling" tab;
 - verify the outermost functions in the top-down view;
 - verify the start function and some of its callees for some threads in the top-down view; 
 - verify the top function in the bottom-up view and some of its callers.
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="triangle.exe"),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="ntdll.dll"),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="kernel32.dll"),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="d3d11.dll"),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="triangle.exe"),
        Capture(),
        VerifySamplingContentForTriangleExe(),
        VerifyTopDownContentForTriangleExe(),
        VerifyBottomUpContentForTriangleExe()
    ]
    suite = E2ETestSuite(test_name="Mixed callstack unwinding", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
