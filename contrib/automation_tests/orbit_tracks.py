"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.capture_window import SelectTrack, DeselectTrack, MoveTrack, FilterTracks, SetEnableAutoFrameTrack, VerifyTracksExist, Capture
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.main_window import EndSession


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="hello_ggp"),
        # Setting enable auto frame track to true.
        SetEnableAutoFrameTrack(enable_auto_frame_track=True),
        # Ending and opening a new session. The auto frame track will appear.
        EndSession(),
        FilterAndSelectFirstProcess(process_filter="hello_ggp"),
        Capture(),
        VerifyTracksExist(
            track_names=["Scheduler", "Frame track*", "gfx", "All Threads", "hello_ggp_stand"]),
        # We check for "sdma0" or "vce0" or both to exist. Both are connected to the encoding of video frames.
        VerifyTracksExist(track_names=[("*sdma0*", "*vce0*")], allow_duplicates=True),
        SelectTrack(track_index=0, expect_failure=True),  # Scheduler track cannot be selected
        FilterTracks(filter_string="hello"),
        SelectTrack(track_index=1),  # hello_ggp_stand track can be selected
        FilterTracks(filter_string=""),
        # TODO(b/233028574): SelectTrack doesn't release the clicked track properly and can't be followed
        # directly with MoveTrack(). It needs to run DeselectTrack() to release the track before moving
        # tracks.
        DeselectTrack(),
        MoveTrack(track_index=4, new_index=0),
        MoveTrack(track_index=0, new_index=3),
        MoveTrack(track_index=3, new_index=4),
        # TODO: The numbers below are very pessimistic, but it's not assured additional tracks like
        # GgpSwapChain, GgpVideoIpcRead etc are present - GgpSwapChain is missing on the DevKit, others
        # depend on the samples that have been taken
        FilterTracks(filter_string="hello", expected_track_count=2),
        FilterTracks(filter_string="Hello", expected_track_count=2)
    ]
    suite = E2ETestSuite(test_name="Track Interaction", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
