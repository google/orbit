"""
Copyright (c) 2022 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""
import logging
import time

from core.common_controls import DataViewPanel
from core.orbit_e2e import E2ETestCase


class VerifySamplingContentForTriangleExe(E2ETestCase):
    """
    Verify that the first two rows of the "(all threads)" sub-tab of the "Sampling" tab correspond to the functions
    `RtlUserThreadStart` (thread start function) and `BaseThreadInitThunk` (calls the thread entry point), in any order.
    """

    def _execute(self):
        self.find_control('TabItem', "Sampling").click_input()
        logging.info("Switched to the 'Sampling' tab")
        tab = self.find_control('Group', 'samplingTab')

        subtab_widget = self.find_control('Group', 'TabWidget', parent=tab)
        thread_subtab = self.find_control('Group', 'SamplingReportThreadTab', parent=subtab_widget)
        sampling_report_dataview = DataViewPanel(
            self.find_control('Group', 'SamplingReportDataView', parent=thread_subtab))

        logging.info(
            "Verifying the functions with the highest inclusive count from the 'Sampling' tab")
        first_function_name = sampling_report_dataview.get_item_at(0, 1).window_text()
        second_function_name = sampling_report_dataview.get_item_at(1, 1).window_text()
        # Allow any order because the two functions will most likely have the same number of inclusive samples.
        self.expect_eq(
            sorted([first_function_name,
                    second_function_name]), ["BaseThreadInitThunk", "RtlUserThreadStart"],
            "Top functions in the 'Sampling' tab are 'RtlUserThreadStart' and 'BaseThreadInitThunk'"
        )
        logging.info(
            "Verified the functions with the highest inclusive count from the 'Sampling' tab")
