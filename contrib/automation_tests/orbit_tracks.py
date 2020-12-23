"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from fragments.tracks import SelectTrack, DeselectTrack, MoveTrack, FilterTracks, MatchTracks
from fragments.capturing import SelectProcess, Capture


def main(argv):
    tests = [SelectProcess(process_search_term="hello_"),
             Capture(length_in_seconds=5),
             MatchTracks(expected_names=["Scheduler", "gfx", "sdma0",
                                         "hello_ggp_stand", "hello_ggp_stand", "GgpSwapchain",
                                         "GgpVideoIpcRead", "GgpVideoIpcRead", "GgpVideoIpc"],
                         allow_additional_tracks=True),
             SelectTrack(track_index=5),
             DeselectTrack(),
             SelectTrack(track_index=0, expect_failure=True),   # Scheduler track cannot be selected
             MoveTrack(track_index=5, new_index=0),
             MoveTrack(track_index=0, new_index=3),
             MoveTrack(track_index=3, new_index=5),
             # TODO: Filtering tracks should probably be tested on a loaded capture...
             FilterTracks(filter_string="hello", expected_count=2),
             FilterTracks(filter_string="Hello", expected_count=2),
             FilterTracks(filter_string="ggp", expected_count=7, allow_additional_tracks=True),
             FilterTracks(filter_string="", expected_count=9, allow_additional_tracks=True)]
    suite = E2ETestSuite(test_name="Track Interaction", tests=tests)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
