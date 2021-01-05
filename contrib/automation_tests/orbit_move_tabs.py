"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from fragments.move_tabs import MoveFunctionsTab


def main(argv):
    tests = [MoveFunctionsTab()]
    suite = E2ETestSuite(test_name="Move tabs", tests=tests)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
