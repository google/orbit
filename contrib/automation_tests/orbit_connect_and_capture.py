"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from fragments.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from fragments.capture_window import Capture


def main(argv):
    tests = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter='hello_'),
        Capture()
    ]
    suite = E2ETestSuite(test_name="Connect & Capture", tests=tests)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
