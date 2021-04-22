"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.capture_window import Capture, FilterTracks
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance

"""Smoke test for the memory tracing and visualization using pywinauto.

This automation script covers a basic workflow:
 - start Orbit, connect to a gamelet, and then select a process
 - enable memory tracing, take a capture and verify the presence of memory tracks
 - disable memory tracing, take a capture and verify that the memory tracks are gone
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter='hello_ggp'),
        Capture(collect_system_memory_usage=True),
        FilterTracks(filter_string="Memory", expected_count=2),
        Capture(),
        FilterTracks(filter_string="Memory", expected_count=0)
    ]
    suite = E2ETestSuite(test_name="Collect System Memory Usage", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
