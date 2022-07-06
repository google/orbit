"""
Copyright (c) 2022 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.capture_window import VerifyTracksExist, Capture
from test_cases.connection_window import WaitForConnectionToTargetInstanceAndProcess
"""
Verifies existence and contents of the ConnectToTargetDialog, and takes a short
capture once the main window appears.

IMPORTANT: Unlike other scripts, this script expects a single unnamed parameter
that specifies the ID of the instance Orbit is expected to connect to.
This is required because the script cannot know the instance name before it is 
reserved by our E2E test infrastructure.

Before this script is run there needs to be a gamelet reserved, and 
"hello_ggp_standalone" has to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers the following workflow:
 - wait for the main window
 - verify the contents of the target label
 - take a short capture
 - verify a track named "hello_ggp_stand" exists
"""


def main(argv):
    if len(argv) < 2:
        raise RuntimeError("Missing argument: You need to pass an unnamed parameter to specify "
                           "the expected instance ID or label.")

    test_cases = [
        WaitForConnectionToTargetInstanceAndProcess(expected_instance_id=argv[1],
                                                    expected_process_path="/mnt/developer/hello_ggp_standalone"),
        Capture(),
        VerifyTracksExist(track_names=["hello_ggp_stand"])
    ]
    suite = E2ETestSuite(test_name="Connect to target", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
