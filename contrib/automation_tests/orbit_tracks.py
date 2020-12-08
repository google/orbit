"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETest
from fragments.capture_window import SelectTrack, MoveTrack


def main(argv):
    fragments = [SelectTrack(-1), MoveTrack(-1)]
    test = E2ETest(test_name="Track Interaction", fragments=fragments)
    test.execute()


if __name__ == '__main__':
    app.run(main)
