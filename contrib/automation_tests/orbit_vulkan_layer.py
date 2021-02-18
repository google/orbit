"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.capture_window import MatchTracks, Capture
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance

"""Basic smoke test for the Vulkan layer functionality using pywinauto.

Before this script is run there needs to be a gamelet reserved and
"benchmark" has to be started with the "--orbit" option.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test it needs
to by run from 64 bit python.

This automation script covers a basic workflow:
 - start Orbit
 - connect to a gamelet
 - select a process
 - take a capture
 - check that there are debug markers for the graphics queue track
"""
def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="benchmark"),
        Capture(),
        MatchTracks(expected_names=["gfx", "gfx_marker"], allow_additional_tracks=True)]
    suite = E2ETestSuite(test_name="Vulkan Layer", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
