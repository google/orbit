"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import (ConnectToStadiaInstance, DisconnectFromStadiaInstance,
                                          RefreshStadiaInstanceList, TestAllInstancesCheckbox,
                                          SelectProjectAndVerifyItHasAtLeastOneInstance,
                                          SelectNextProject)
"""Test Instance connections in the Session Setup Window 

Before this script is run there needs to be a gamelet reserved.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script does the following:
 - Connect to the first instance in the list
 - Disconnect from that instance and verify correct overlay behaviour and 
    that instances exist after the disconnect
 - Click on the refresh button and verify correct overlay behaviour and
    that instances exist after the refresh
"""


def main(argv):
    test_cases = [
        SelectProjectAndVerifyItHasAtLeastOneInstance(
            project_name="Default Project (Automated Testing)"),
        SelectNextProject(),
        SelectProjectAndVerifyItHasAtLeastOneInstance(
            project_name="Default Project (Automated Testing)"),
        TestAllInstancesCheckbox(),
        ConnectToStadiaInstance(),
        DisconnectFromStadiaInstance(),
        RefreshStadiaInstanceList()
    ]
    suite = E2ETestSuite(test_name="Instance Connections", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)