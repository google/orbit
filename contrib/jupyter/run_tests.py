"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import orbitutils.run_all_unittests as unit_tests
import sys

def main():
    print("Running Python unit tests")
    return unit_tests.main()

if __name__ == "__main__":
    sys.exit(main())
