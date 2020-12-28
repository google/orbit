"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from core.orbit_e2e import E2ETestCase
from orbit_testing import select_process, capture, focus_on_capture_window


class SelectProcess(E2ETestCase):
    def _execute(self, process_search_term="hello_"):
        select_process(self.suite.application, process_search_term=process_search_term)


class Capture(E2ETestCase):
    def _execute(self, length_in_seconds):
        focus_on_capture_window(self.suite.application)
        capture(self.suite.application, length=length_in_seconds)
