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
import orbit_testing
import logging
import time
from absl import app
import pywinauto
from pywinauto.application import Application

def main(argv):
  orbit_testing.WaitForOrbit()
  application = Application(backend='uia').connect(title_re='orbitprofiler')
  orbit_testing.ConnectToGamelet(application)
  orbit_testing.SelectProcess(application, 'hello_')
  orbit_testing.LoadSymbols(application, 'hello_')
  orbit_testing.HookFunction(application, 'DrawFrame')
  orbit_testing.FocusOnCaptureWindow(application)
  orbit_testing.Capture(application, 5);

  main_wnd = application.window(title_re='orbitprofiler', found_index=0)

  # Check the output in the live tab. DrawFrames should have been called ~300
  # times (60 Hz * 5 seconds).
  children = main_wnd.TreeView.children()
  for i in range(len(children)):
    if 'DrawFrame' in children[i].window_text():
      num = int(children[i + 1].window_text())
      if num < 30 or num > 3000:
        raise RuntimeError('Wrong number of calls to "DrawFrame": ' + str(num))
      else:
        logging.info('Verified number of calls to "DrawFrame".')

  main_wnd.CloseButton.click_input()
  logging.info('Closed Orbit.')


if __name__ == '__main__':
  app.run(main)
