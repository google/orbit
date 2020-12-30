"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app, flags

from core.orbit_e2e import E2ETestSuite
from fragments.connection_window import ConnectToStadiaInstance, FilterAndSelectFirstProcess
from fragments.main_window import MoveTab


def main(argv):
    tab_title = "Symbols" if flags.FLAGS.enable_ui_beta else "Functions"
    tests = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="hello_ggp"),
        MoveTab(tab_title=tab_title, tab_name="FunctionsTab")
    ]
    suite = E2ETestSuite(test_name="Move tabs", tests=tests)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
