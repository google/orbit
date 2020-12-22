"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETest
from fragments.tracks import SelectTrack, DeselectTrack, MoveTrack, FilterTracks, MatchTracks


def main(argv):
    fragments = [MatchTracks(expected_count=9,
                             expected_names=["Scheduler", "gfx", "sdma0",
                                             "hello_ggp_stand", "hello_ggp_stand", "GgpSwapchain",
                                             "GgpVideoIpcRead", "GgpVideoIpcRead", "GgpVideoIpc"]),
                 SelectTrack(track_index=-1),
                 DeselectTrack(),
                 SelectTrack(track_index=0, expect_failure=True),   # Scheduler track cannot be selected
                 MoveTrack(track_index=-1, new_index=0),
                 MoveTrack(track_index=0, new_index=3),
                 MoveTrack(track_index=3, new_index=-1),
                 FilterTracks(filter_string="hello", expected_count=2),
                 FilterTracks(filter_string="Hello", expected_count=2),
                 FilterTracks(filter_string="ggp", expected_count=6),
                 FilterTracks(filter_string="", expected_count=9)]
    test = E2ETest(test_name="Track Interaction", fragments=fragments)
    test.execute()


if __name__ == '__main__':
    app.run(main)
