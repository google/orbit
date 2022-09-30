"""
Copyright (c) 2022 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.capture_log import VerifyCaptureLogContains
from test_cases.capture_window import Capture, CheckTimers
from test_cases.symbols_tab import WaitForLoadingSymbolsAndCheckModuleState, FilterAndHookFunction
from test_cases.live_tab import VerifyScopeTypeAndHitCount
"""Instrument functions using uprobes on Wine.

Before this script is run there needs to be a gamelet reserved and our version
of "triangle.exe" with the `Render` function made `__declspec(noinline)` has to
be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - connect to a gamelet, select a process running on Wine, load debug symbols
 - instrument a function from a PE with FileAlignment multiple of the page size
 - instrument a function from a PE with FileAlignment lower than the page size
 - take a capture using uprobes for dynamic instrumentation
 - verify the first hooked function is recorded
 - verify the second function has no hits and gives a corresponding warning in the Capture Log
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="triangle.exe"),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="d3d11.dll"),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="triangle.exe"),
        FilterAndHookFunction(
            function_search_string="dxvk::vk::Presenter::presentImage{ }d3d11.dll"),
        FilterAndHookFunction(function_search_string="Render{ }triangle.exe"),
        Capture(user_space_instrumentation=False),
        CheckTimers(track_name_filter="dxvk-submit"),
        VerifyScopeTypeAndHitCount(scope_name="dxvk::vk::Presenter::presentImage",
                                   scope_type="D",
                                   min_hits=700,
                                   max_hits=70000),
        VerifyScopeTypeAndHitCount(scope_name="Render", scope_type="D", min_hits=0, max_hits=0),
        VerifyCaptureLogContains(expected_content=[
            "Uprobes likely failed to instrument some functions:",
            "Function \"Render()\" belonging to module \"/mnt/developer/d3d11_triangle_orbit_api/triangle.exe\" is not (always) loaded into a file mapping."
        ])
    ]
    suite = E2ETestSuite(test_name="Uprobes on Wine", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
