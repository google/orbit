"""
Copyright (c) 2022 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""
import logging
from typing import Iterable

from core.orbit_e2e import E2ETestCase


class VerifyCaptureLogContains(E2ETestCase):
    """
    Verify that all the listed strings appear in the Capture Log.
    """

    def _execute(self, expected_content: str or Iterable[str]):
        if isinstance(expected_content, str):
            expected_content = [expected_content]

        capture_log_button = self.find_control('CheckBox', 'CaptureLogButton')
        if not capture_log_button.get_toggle_state():
            logging.info("Showing the Capture Log")
            capture_log_button.click_input()

        capture_log_text_edit = self.find_control('Edit', 'CaptureLogTextEdit')
        capture_log_content = capture_log_text_edit.window_text()

        for expected_string in expected_content:
            logging.info(f"Looking for '{expected_string}' in Capture Log")
            self.expect_true(expected_string in capture_log_content,
                             f"'{expected_string}' is in Capture Log")
