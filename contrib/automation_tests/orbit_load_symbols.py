"""
Copyright (c) 2022 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.symbols_tab import ClearSymbolCache, LoadSymbols, MODULE_STATE_PARTIAL, VerifySymbolsLoaded, \
    WaitForLoadingSymbolsAndCheckModuleState, WaitForLoadingSymbolsAndCheckNoErrorStates
from test_cases.symbol_locations import ClearAllSymbolLocations
"""
Test symbol loading.

Before this script is run there needs to be a gamelet reserved and
"no_symbols_elf" has to be started (can be downloaded from GCS).

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - connect to a gamelet
 - clear symbol cache and delete all symbol locations
 - verify no modules have symbols in the "Error" state
 - verify symbols for "libc" are in the "Loaded" state, and functions are present
 - verify symbols for "no_symbols_elf" are in the "Partial" state, and functions are present
 - verify manual symbol loading for "no_symbols_elf" is possible but fails
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter='no_symbols_elf'),
        ClearSymbolCache(),
        ClearAllSymbolLocations(),
        WaitForLoadingSymbolsAndCheckNoErrorStates(),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="libc"),
        VerifySymbolsLoaded(symbol_search_string="fprintf{ }libc"),
        WaitForLoadingSymbolsAndCheckModuleState(module_search_string="no_symbols_elf",
                                                 expected_state=MODULE_STATE_PARTIAL),
        VerifySymbolsLoaded(
            symbol_search_string="[function@0x2974]{ }no_symbols_elf"),  # Corresponds to `main`.
        LoadSymbols(module_search_string="no_symbols_elf", expect_fail=True),
    ]
    suite = E2ETestSuite(test_name="Automatic symbol loading", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
