"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.capture_window import VerifyTracksExist, Capture, CheckTimers, ExpandTrack, CollapseTrack
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
"""Basic smoke test for the Vulkan layer functionality using pywinauto.

Before this script is run there needs to be a gamelet reserved and
"UE4Game.elf" has to be started with the "--orbit" option.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - connect to a gamelet
 - select a process
 - take a capture
 - check that there are debug markers for the graphics queue track
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="UE4Game"),
        Capture(),
        VerifyTracksExist(track_names=["gfx"]),
        ExpandTrack(expected_name="gfx"),
        CheckTimers(track_name_filter='gfx_submissions', recursive=True),
        CollapseTrack(expected_name='gfx_submissions', recursive=True),
        CheckTimers(track_name_filter='gfx_marker', recursive=True),
        CollapseTrack(expected_name="gfx")
    ]
    suite = E2ETestSuite(test_name="Vulkan Layer", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
