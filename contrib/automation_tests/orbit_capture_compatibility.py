"""
Copyright (c) 2022 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""
from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.capture_window import Capture
from test_cases.connection_window import ConnectToStadiaInstance, FilterAndSelectFirstProcess, LoadCapture
from test_cases.live_tab import VerifyOneFunctionWasHit
from test_cases.main_window import DismissDialog, RenameMoveCaptureFile
from test_cases.symbols_tab import WaitForLoadingSymbolsAndCheckModuleState
"""Verify Capture compatibility

This script contains functionality to record a capture with one (new) version of Orbit and then
test that this capture can be loaded with another (old) version of Orbit.

This needs to be called with one of two arguments: 1) one of `record_capture` or `load_capture`,
and 2) a path to where the capture will be saved / from where it will be loaded.

Before the record part of this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started. Further, (new) Orbit needs to be started.

For the load part of this script, (old) Orbit needs to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.
"""


def main(argv):
    if len(argv) < 3:
        raise RuntimeError(
            "Missing argument: You need to pass 2 arguments. The first is `record_capture` or "
            "`load_capture` and the second a path to save/load the capture file"
        )
    if argv[1] == "record_capture":
        test_cases = [
            ConnectToStadiaInstance(),
            FilterAndSelectFirstProcess(process_filter='hello_'),
            WaitForLoadingSymbolsAndCheckModuleState(module_search_string="hello_ggp"),
            Capture(length_in_seconds=5),
            VerifyOneFunctionWasHit(function_name_contains="vulkan"),
            RenameMoveCaptureFile(new_capture_path=argv[2])
        ]
    elif argv[1] == "load_capture":
        test_cases = [
            LoadCapture(capture_file_path=argv[2]),
            DismissDialog(title_contains="Capture"),  # This dismisses the old version warning
            VerifyOneFunctionWasHit(function_name_contains="vulkan")
        ]
    else:
        raise RuntimeError(
            "Wrong value for first argument (allowed values: record_capture, load_capture): " +
            argv[1])

    suite = E2ETestSuite(test_name="Verify Capture compatibility", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
