"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.capture_window import Capture, FilterTracks, CheckTimers, SetEnableAutoFrameTrack, VerifyTracksExist
from test_cases.main_window import EndSession
from test_cases.symbols_tab import WaitForLoadingSymbolsAndCheckModuleState, FilterAndHookFunction
from test_cases.live_tab import AddFrameTrack
"""Create a frame track in Orbit using pywinauto.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from  64 bit python.

This automation script covers a basic workflow:
 - connect to a gamelet
 - select a process and load debug symbols
 - instrument a function
 - take a capture
 - in the "Live" tab, create a frame track for the instrumented function
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="hello_ggp"),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="hello_ggp"),
        # Setting enable auto frame track to false to only have the created Frame Track.
        SetEnableAutoFrameTrack(enable_auto_frame_track=False),
        # Ending and opening a new session. The auto frame track won't appear.
        EndSession(),
        FilterAndSelectFirstProcess(process_filter="hello_ggp"),
        FilterAndHookFunction(function_search_string='DrawFrame'),
        Capture(),
        AddFrameTrack(function_name="DrawFrame"),
        VerifyTracksExist(track_names="Frame track*"),
        CheckTimers(track_name_filter='Frame track*')  # Verify the frame track has timers
    ]
    suite = E2ETestSuite(test_name="Add Frame Track", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
