"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import unittest
import orbitutils.orbit_capture

class TestOrbitCapture(unittest.TestCase):
    def test_opening_capture(self):
        with open('./orbitutils/testdata/OrbitTest_capture.orbit', 'rb') as f:
            capture = orbitutils.orbit_capture.OrbitCapture(f.read())
            min_ns = capture.compute_min_time_ns()
            max_ns = capture.compute_max_time_ns()
      
            self.assertEqual(31678, len(capture.timer_infos))
            self.assertEqual(1200163245628561, min_ns)
            self.assertEqual(1200166685446087, max_ns)

if __name__ == '__main__':
    unittest.main()