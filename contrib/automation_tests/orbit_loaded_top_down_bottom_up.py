"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.bottom_up_tab import VerifyBottomUpContentForLoadedCapture
from test_cases.capture_window import FilterTracks, SelectAllCallstacks, ZoomOut
from test_cases.connection_window import LoadCapture
from test_cases.top_down_tab import VerifyTopDownContentForLoadedCapture
"""Inspect the top-down view in Orbit using pywinauto.

Before this script is run Orbit needs to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - load a known capture
 - verify the content of the top-down view
 - verify the content of the bottom-up view
 - select all callstacks using the "(all threads)" track
 - verify the content of the top-down view for the selection
 - verify the content of the bottom-up view for the selection
"""


def main(argv):
    test_cases = [
        LoadCapture(capture_file_path="testdata\\OrbitTest_1-72.orbit"),
        VerifyTopDownContentForLoadedCapture(selection_tab=False),
        VerifyBottomUpContentForLoadedCapture(selection_tab=False),

        ZoomOut(),
        FilterTracks(filter_string='threads{)}'),
        SelectAllCallstacks(track_name_filter='All Threads'),
        VerifyTopDownContentForLoadedCapture(selection_tab=True),
        VerifyBottomUpContentForLoadedCapture(selection_tab=True),
    ]
    suite = E2ETestSuite(test_name="Top-Down and Bottom-Up view of loaded capture", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
