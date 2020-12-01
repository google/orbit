"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETest
from fragments.move_tabs import MoveFunctionsTab


def main(argv):
    fragments = [MoveFunctionsTab()]
    test = E2ETest(test_name="Move tabs", fragments=fragments)
    test.execute()


if __name__ == '__main__':
    app.run(main)
