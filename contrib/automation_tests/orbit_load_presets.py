"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.symbols_tab import LoadAndVerifyHelloGgpPreset, WaitForLoadingSymbolsAndVerifyCache
"""Apply two presets in Orbit using pywinauto.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started. Two presets named
draw_frame_in_hello_ggp_1_68.opr and ggp_issue_frame_token_in_hello_ggp_1_68.opr
(hooking the functions DrawFrame and GgpIssueFrameToken) need to exist in the
preset folder.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from  64 bit python.
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="hello_ggp"),
        WaitForLoadingSymbolsAndVerifyCache(),
        LoadAndVerifyHelloGgpPreset()
    ]
    suite = E2ETestSuite(test_name="Load Preset", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
