"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

"""Create a frame track in Orbit using pywinauto.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test it needs
to by run from  64 bit python.

This automation script covers a basic workflow:
 - start Orbit
 - connect to a gamelet
 - select a process and load debug symbols
 - instrument a function
 - take a capture
 - in the "Live" tab, create a frame track for the instrumented function
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
  orbit_testing.Capture(application, 5)

  main_wnd = application.window(title_re='orbitprofiler', found_index=0)
  # Find 'DrawFrame' in the live tab and create a frame track.
  children = main_wnd.TreeView.children()
  for i in range(len(children)):
    if 'DrawFrame' in children[i].window_text():
      children[i].click_input(button='right')
      main_wnd.child_window(title='Add frame track(s)', control_type="MenuItem").click_input()

  # Since the frame track is only a visualization and only affects the 
  # OpenGL rendered capture window, there is currently no way to verify
  # automatically that the frame track was created correctly.

  main_wnd.CloseButton.click_input()
  logging.info('Closed Orbit.')


if __name__ == '__main__':
  app.run(main)
