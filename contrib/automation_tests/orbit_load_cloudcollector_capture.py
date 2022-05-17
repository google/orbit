"""
Copyright (c) 2022 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""
from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.capture_window import VerifyTracksExist
from test_cases.connection_window import LoadCapture

"""Verify loading a capture taken by CloudCollector in Orbit using pywinauto.

Before this script is run, Orbit has to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - load a capture for hello_ggp taken by the CloudCollector from the provided path
 - verify that the capture contains tracks
"""


def main(argv):
    test_cases = [
        LoadCapture(capture_file_path="C:\\Users\\yrunner\\Documents\\Orbit\\captures\\CloudCollectorTest.orbit"),
        VerifyTracksExist(track_names="hello_ggp_stand*", allow_duplicates=True)
    ]
    suite = E2ETestSuite(test_name="Load CloudCollector Capture", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
