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
 - start Orbit, connect to a gamelet, and then select a process
 - enable memory tracing, set and validate the memory sampling period
 - enable memory tracing, take a capture and verify the presence of memory tracks & pagefault track
 - disable memory tracing, take a capture and verify that memory tracks & pagefault track are gone
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
        VerifyTracksExist(track_names="Pagefault*"),
        CollapseTrack(expected_name="Pagefault Track"),
        ExpandTrack(expected_name="Pagefault Track"),
        VerifyTracksExist(track_names="System Memory*"),
        CollapseTrack(expected_name="System Memory Usage (MB)"),
        ExpandTrack(expected_name="System Memory Usage (MB)"),
        VerifyTracksExist(track_names="CGroup Memory*"),
        CollapseTrack(expected_name="CGroup Memory Usage (MB)"),
        ExpandTrack(expected_name="CGroup Memory Usage (MB)"),
        Capture(),
        VerifyTracksDoNotExist(track_names="Pagefault*"),
        VerifyTracksDoNotExist(track_names="*Memory*")
    ]
    suite = E2ETestSuite(test_name="Collect Memory Usage & Pagefault Information", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
