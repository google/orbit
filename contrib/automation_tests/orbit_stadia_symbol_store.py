"""
Copyright (c) 2022 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.symbols_tab import ClearSymbolCache, LoadSymbols, WaitForLoadingSymbolsAndCheckModuleState
from test_cases.symbol_locations import ClearAllSymbolLocations, ToggleEnableStadiaSymbolStore
from test_cases.main_window import EndSession
"""
Test symbol loading from Stadia symbol store.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started. 
Also Orbit needs to be started with the command line argument "--disable_instance_symbols".

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - (pre clean up) connect to the gamelet and start session, 
   disable stadia symbol store, clear the symbol cache and delete all symbol locations
 - restart session
 - select the process "hello_ggp_standalone"
 - verify symbol loading fails
 - enable stadia symbol store
 - verify symbol loading works
 - (post clean up) disable stadia symbol store
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter='hello_ggp'),
        ToggleEnableStadiaSymbolStore(enable_stadia_symbol_store=False),
        ClearAllSymbolLocations(),
        ClearSymbolCache(),
        EndSession(),
        FilterAndSelectFirstProcess(process_filter='hello_ggp'),
        LoadSymbols(module_search_string="libggp", expect_fail=True),
        ToggleEnableStadiaSymbolStore(enable_stadia_symbol_store=True),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="libggp"),
        ToggleEnableStadiaSymbolStore(enable_stadia_symbol_store=False)
    ]
    suite = E2ETestSuite(test_name="Stadia symbol store", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
