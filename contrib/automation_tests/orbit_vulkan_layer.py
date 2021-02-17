"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.capture_window import MatchTracks, Capture
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance

"""
Note that this test is based on Trata (rather then hello_ggp).
"""
def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="benchmark"),
        Capture(),
        MatchTracks(expected_names=["gfx", "gfx_marker"], allow_additional_tracks=True)]
    suite = E2ETestSuite(test_name="Track Interaction", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
