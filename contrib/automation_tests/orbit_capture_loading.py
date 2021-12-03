"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""
from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import LoadCapture
from test_cases.live_tab import AddIterator, VerifyFunctionCallCount
from test_cases.capture_window import CheckTimers, FilterTracks, CheckThreadStates, ToggleCollapsedStateOfAllTracks, \
    VerifyTracksExist

"""Verify loading a capture in Orbit using pywinauto.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started. Also Orbit needs to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test it needs
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
"""


def main(argv):
    test_cases = [
        LoadCapture(capture_file_path="test_data\\OrbitTest_1-64.orbit", expect_fail=True),
        LoadCapture(capture_file_path="test_data\\OrbitTest_1-72.orbit"),

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
        VerifyFunctionCallCount(function_name='TestFunc2', min_calls=1257, max_calls=1257)
    ]
    suite = E2ETestSuite(test_name="Capture Loading", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
