"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""
"""Instrument a single function in Orbit using pywinauto.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test it needs
to by run from 64 bit python.

This automation script covers a basic workflow:
 - start Orbit
 - connect to a gamelet
 - select a process and load debug symbols
 - instrument a function
 - take a capture and verify the hooked function is recorded
"""
import logging

import orbit_testing
from absl import app
from pywinauto.application import Application


def main(argv):
    orbit_testing.wait_for_orbit()
    application = Application(backend='uia').connect(title_re='orbitprofiler')
    orbit_testing.connect_to_gamelet(application)
    orbit_testing.select_process(application, 'hello_')
    orbit_testing.load_symbols(application, 'hello_')
    orbit_testing.hook_function(application, 'DrawFrame')
    orbit_testing.focus_on_capture_window(application)
    orbit_testing.capture(application, 5)

    main_wnd = application.window(title_re='orbitprofiler', found_index=0)

    # Check the output in the live tab. DrawFrames should have been called ~300
    # times (60 Hz * 5 seconds).
    children = main_wnd.TreeView.children()
    for i in range(len(children)):
        if 'DrawFrame' in children[i].window_text():
            num = int(children[i + 1].window_text())
            if num < 30 or num > 3000:
                raise RuntimeError('Wrong number of calls to "DrawFrame": ' +
                                   str(num))
            else:
                logging.info('Verified number of calls to "DrawFrame".')

    main_wnd.CloseButton.click_input()
    logging.info('Closed Orbit.')


if __name__ == '__main__':
    app.run(main)
