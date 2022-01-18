"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from absl import app

from core.orbit_e2e import E2ETestSuite
from test_cases.connection_window import FilterAndSelectFirstProcess, ConnectToStadiaInstance
from test_cases.symbols_tab import LoadAllSymbolsAndVerifyCache, ClearSymbolCache, LoadSymbols, \
    ForceAndVerifySymbolUpdate
from test_cases.main_window import EndSession

"""
Test symbol loading with and without local caching.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started. 

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test, it needs
to be run from 64 bit python.

This automation script covers a basic workflow:
 - connect to a gamelet
 - delete the symbol cache
 - load all symbols, store loading times, verify symbol files exist in cache
 - restart session
 - load all symbols again, loading times should have decreased. (This has a *very* high threshold for failure
    as we can't rely on the timing on the test infrastructure. As long as it's 1% faster, it passes.)
 - restart session
 - load libggp, measure loading time (we can't measure a single module loading time before)
 - invalidate symbols for libggp by replacing the cache file with another file
 - restart session
 - load libggp, loading time should have increased (as above, as long as it's 5% faster, it passes.)
"""


def main(argv):
    test_cases = [
        ConnectToStadiaInstance(),
        FilterAndSelectFirstProcess(process_filter='hello_ggp'),
        ClearSymbolCache(),
        LoadAllSymbolsAndVerifyCache(),
        EndSession(),
        FilterAndSelectFirstProcess(process_filter='hello_ggp'),
        LoadAllSymbolsAndVerifyCache(expected_duration_difference_ratio=0.99),
        EndSession(),
        FilterAndSelectFirstProcess(process_filter='hello_ggp'),
        LoadSymbols(module_search_string="libggp"),
        EndSession(),
        FilterAndSelectFirstProcess(process_filter='hello_ggp'),
        ForceAndVerifySymbolUpdate(full_module_path="/user/local/cloudcast/lib/libggp.so",
                                   replace_with_module="/mnt/developer/hello_ggp_standalone")

    ]
    suite = E2ETestSuite(test_name="Symbol loading and caching", test_cases=test_cases)
    suite.execute()


if __name__ == '__main__':
    app.run(main)
