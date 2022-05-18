"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""
from absl import app
from datetime import date, timedelta

from core.orbit_e2e import E2ETestSuite
from test_cases.capture_window import Capture, CheckTimers, CheckThreadStates, FilterTracks, \
    ToggleCollapsedStateOfAllTracks, VerifyTracksExist
from test_cases.connection_window import ConnectToStadiaInstance, FilterAndSelectFirstProcess, LoadCapture, \
    LoadLatestCapture
from test_cases.live_tab import AddIterator, VerifyScopeTypeAndHitCount
from test_cases.main_window import EndSession
"""Verify loading a capture in Orbit using pywinauto.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started. Further, Orbit needs to be started.
Also, the captures directory should be cleared.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - load an old, unsupported capture and verify this fails with a message
 - load a supported capture
 - verify that the scheduler track is present and contains timers
 - verify that the frame track is present and contains timers
 - verify that the tracks from manual instrumentation are present
 - verify that the memory tracks are present
 - verify that an iterator can be added to "TestFunc2"
 - verify that "TestFunc2" was called exactly 1257 times
 - take a capture and verify there is a corresponding capture in the latest captures list which contains the tracks
"""


def main(argv):
    # During the tests, we want to verify that captures get automatically saved. We will do so by filtering the recent
    # captures list with the current date (in addition to also deleting old captures before this script runs). However,
    # if it is around midnight when this code gets executed and we store the date string, it can be that the capture
    # actually gets taken on the next day. Therefore, we will also check for the next day.
    today = date.today()
    tomorrow = today + timedelta(days=1)
    today_string = today.strftime("%Y_%m_%d")
    tomorrow_string = tomorrow.strftime("%Y_%m_%d")
    test_cases = [
        LoadCapture(capture_file_path="testdata\\OrbitTest_1-64.orbit", expect_fail=True),
        LoadCapture(capture_file_path="testdata\\OrbitTest_1-72.orbit"),
        FilterTracks(filter_string="Scheduler", expected_track_count=1),
        CheckTimers(track_name_filter='Scheduler*'),
        FilterTracks(filter_string="Frame", expected_track_count=1),
        CheckTimers(track_name_filter='Frame track*'),  # Verify the frame track has timers
        FilterTracks(filter_string="DynamicName_", expected_track_count=5),
        FilterTracks(filter_string="_var", expected_track_count=6),
        FilterTracks(filter_string="OrbitThread_", expected_track_count=1),
        ToggleCollapsedStateOfAllTracks(),
        CheckTimers(track_name_filter="OrbitThread_*"),
        CheckThreadStates(track_name_filter='OrbitThread_*'),
        FilterTracks(filter_string="ORBIT_ASYNC_TASKS", expected_track_count=1),
        CheckTimers(track_name_filter="ORBIT_ASYNC_TASKS"),
        FilterTracks(filter_string="ORBIT_START_ASYNC_TEST", expected_track_count=1),
        CheckTimers(track_name_filter="ORBIT_START_ASYNC_TEST"),
        FilterTracks(filter_string=""),
        VerifyTracksExist(track_names=["Page*", "*System*", "*CGroup*"], allow_duplicates=True),
        AddIterator(function_name="TestFunc2"),
        VerifyScopeTypeAndHitCount(scope_name="TestFunc2",
                                   scope_type="D",
                                   min_hits=1257,
                                   max_hits=1257),

        # Let's take a capture with the current version and verify this can be loaded
        EndSession(),
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="hello_ggp"),
        Capture(),
        VerifyTracksExist(track_names="hello_ggp_stand*", allow_duplicates=True),
        EndSession(),
        # If we took the capture around midnight, we need to ensure to also look for the next day. Remember, the strings
        # get created before the tests run. Thus the `today_string` might be actually from the day before the capture
        # gets auto-saved.
        LoadLatestCapture(filter_strings=[
            f"hello_ggp_stand_{today_string}", f"hello_ggp_stand_{tomorrow_string}"
        ]),
        VerifyTracksExist(track_names="hello_ggp_stand*", allow_duplicates=True)
    ]
    suite = E2ETestSuite(test_name="Capture Loading", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
