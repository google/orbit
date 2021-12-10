"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.capture_window import Capture, VerifyTracksExist, SetAndCheckMemorySamplingPeriod, \
    VerifyTracksDoNotExist, ExpandTrack, CollapseTrack
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
"""Smoke test for the memory tracing and visualization using pywinauto.

This automation script covers a basic workflow:
 - connect to a gamelet, and then select a process
 - enable memory tracing, set and validate the memory sampling period
 - enable memory tracing, take a capture and verify the presence of memory tracks & page faults track
 - disable memory tracing, take a capture and verify that memory tracks & page faults track are gone
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter='hello_ggp'),
        SetAndCheckMemorySamplingPeriod(memory_sampling_period='030'),
        SetAndCheckMemorySamplingPeriod(memory_sampling_period=''),
        SetAndCheckMemorySamplingPeriod(memory_sampling_period='ab'),
        SetAndCheckMemorySamplingPeriod(memory_sampling_period='0'),
        Capture(collect_system_memory_usage=True),
        VerifyTracksExist(track_names="Page Faults*"),
        ExpandTrack(expected_name="Page Faults"),
        CollapseTrack(expected_name="Page Faults"),
        VerifyTracksExist(track_names="Memory Usage: System*"),
        ExpandTrack(expected_name="Memory Usage: System (MB)"),
        CollapseTrack(expected_name="Memory Usage: System (MB)"),
        VerifyTracksExist(track_names="Memory Usage: CGroup*"),
        ExpandTrack(expected_name="Memory Usage: CGroup (MB)"),
        CollapseTrack(expected_name="Memory Usage: CGroup (MB)"),
        Capture(),
        VerifyTracksDoNotExist(track_names="Page Faults*"),
        VerifyTracksDoNotExist(track_names="*Memory Usage*")
    ]
    suite = E2ETestSuite(test_name="Collect Memory Usage & Page Faults Information",
                         test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
