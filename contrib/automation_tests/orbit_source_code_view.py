"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app
from absl import flags

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess
from test_cases.connection_window import ConnectToStadiaInstance
from test_cases.symbols_tab import WaitForLoadingSymbolsAndCheckModuleState
from test_cases.symbols_tab import ShowSourceCode
"""Show source code for a single function in Orbit using pywinauto.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_c" has to be started. Additionally "hello_ggp_c.debug" needs
to be in Orbit's symbol path. A precompiled version of the hello_ggp
project can be found in the integration test bucket (authentication needed):
https://storage.cloud.google.com/orbit-integration-test-data/hello_ggp.zip
In addition to the executable, also the source code file "main.c" from the
example is needed. The path to this file can be given by the command line
argument "--source_code_file". It needs to be an absolute path!

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - connect to a gamelet
 - select a process and load debug symbols
 - choose "Go to Source code" for a function
 - set up source code mapping
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter="hello_ggp_c"),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="hello_ggp_c"),
        ShowSourceCode(function_search_string="DrawFrame"),
    ]
    suite = E2ETestSuite(test_name="Show Source Code", test_cases=test_cases)
    suite.execute()


# The default path works for E2E test machines!
flags.DEFINE_string("source_code_file", "C:\\build\\scratch\\test\\hello_ggp\\src\\main.c",
                    "An absolute path to main.c from hello_ggp.")

if __name__ == '__main__':
    app.run(main)
