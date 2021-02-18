"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import sys
import unittest

def main():
    tests = unittest.TestLoader().discover('.', '*test.py', '.')
    result = unittest.TextTestRunner().run(tests)
    if result.wasSuccessful():
        return 0
    else:
        return 1

if __name__ == '__main__':
    sys.exit(main())