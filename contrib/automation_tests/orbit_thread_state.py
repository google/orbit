"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.capture_window import Capture, CheckThreadStates
"""Smoke test for thread state collection.

This automated test takes a capture on "hello_ggp_standalone" with thread state
collection enabled and verifies the presence of at least one track in the
capture window.
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter='hello_'),
        Capture(collect_thread_states=True),
        CheckThreadStates(track_name_filter='hello_ggp_stand'),
        Capture(collect_thread_states=False),
        CheckThreadStates(track_name_filter='hello_ggp_stand', expect_exists=False),
    ]
    suite = E2ETestSuite(test_name="Collect Thread States", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
