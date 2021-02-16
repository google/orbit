"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import ConnectToStadiaInstance, FilterAndSelectFirstProcess
from test_cases.main_window import MoveTab


def main(argv):
    tab_title = "Symbols"
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="hello_ggp"),
        MoveTab(tab_title=tab_title, tab_name="SymbolsTab")
    ]
    suite = E2ETestSuite(test_name="Move tabs", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
