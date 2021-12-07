"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import os

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.symbols_tab import ClearSymbolCache, LoadSymbols
from test_cases.symbol_locations import AddSymbolLocation, ClearAllSymbolLocations

"""
Test symbol loading from custom symbol locations.

Before this script is run there needs to be a gamelet reserved and
"no_symbols_elf" has to be started (can be downloaded from GCS). 
Test data needs to be available in  "testdata/symbol_location_tests" (part of our git repo).

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - connect to a gamelet
 - delete all symbol locations
 - verify symbol loading for "no_symbols_elf" fails
 - add a custom symbol location with a stale symbol file
 - verify symbol loading still fails
 - add a symbol location with a correct symbol file
 - verify symbol loading works
 - delete all symbol locations again
"""

stale_path = os.path.abspath(os.path.join(os.path.dirname(__file__),
                                          r"testdata\symbol_location_tests\stale_symbols"))
working_path = os.path.abspath(os.path.join(os.path.dirname(__file__),
                                            r"testdata\symbol_location_tests\working_symbols"))


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter='no_symbols_elf'),
        ClearSymbolCache(),
        ClearAllSymbolLocations(),
        LoadSymbols(module_search_string="no_symbols_elf", expect_fail=True),
        AddSymbolLocation(location=stale_path),
        LoadSymbols(module_search_string="no_symbols_elf", expect_fail=True),
        AddSymbolLocation(location=working_path),
        LoadSymbols(module_search_string="no_symbols_elf"),
        ClearAllSymbolLocations()
    ]
    suite = E2ETestSuite(test_name="Custom symbol locations", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
