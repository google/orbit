"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

"""Apply two presets in Orbit using pywinauto.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started. Two presets named
draw_frame_in_hello_ggp_1_52.opr and ggp_issue_frame_token_in_hello_ggp_1_52
(hooking the functions DrawFrame and GgpIssueFrameToken) need to exist in the
preset folder.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test it needs
to by run from  64 bit python.
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

  # Load the presets.
  main_wnd = application.window(title_re='orbitprofiler', found_index=0)
  main_wnd.PresetsTreeView2.draw_frame_in_hello_ggp_1_52.click_input(
      button='right')
  main_wnd.LoadPreset.click_input()
  logging.info('Loaded Preset DrawFrame')
  main_wnd.PresetsTreeView2.ggp_issue_frame_token_in_hello_ggp_1_52.click_input(
      button='right')
  main_wnd.LoadPreset.click_input()
  logging.info('Loaded Preset GgpIssueFrameToken.')

  # Check that the check mark in front of the hooked function is there.
  # TreeView is the panel that contains all the function names.
  main_wnd.FunctionsTabItem.click_input()
  children = main_wnd.TreeView.children()
  found_draw_frame = False
  found_ggp_issue_frame_token = False
  for i in range(len(children)):
    if 'DrawFrame' in children[i].window_text():
      found_draw_frame = True
      if  u'\u2713' not in children[i - 1].window_text():
        raise RuntimeError(
            'Function DrawFrame was not hooked by appling preset.' + str(children[i - 1].window_text()))
      else:
        logging.info('Verified DrawFrame was hooked.')
    if 'GgpIssueFrameToken' in children[i].window_text():
      found_ggp_issue_frame_token = True
      if  u'\u2713' not in children[i - 1].window_text():
        raise RuntimeError(
            'Function GgpIssueFrameToken was not hooked by appling preset.')
      else:
        logging.info('Verified GgpIssueFrameToken was hooked.')

  if not found_draw_frame:
    raise RuntimeError('Function DrawFrame was not found in the binary.')
  if not found_ggp_issue_frame_token:
    raise RuntimeError(
        'Function GgpIssueFrameToken was not found in the binary.')


  orbit_testing.FocusOnCaptureWindow(application)
  orbit_testing.Capture(application, 5)

  # Check the output in the live tab. DrawFrames should have been called ~300
  # times (60 Hz * 5 seconds).
  children = main_wnd.TreeView.children()
  found_draw_frame = False
  found_ggp_issue_frame_token = False
  for i in range(len(children)):
    if 'DrawFrame' in children[i].window_text():
      found_draw_frame = True
      num = int(children[i + 1].window_text())
      if num < 30 or num > 3000:
        raise RuntimeError('Wrong number of calls to "DrawFrame": ' + str(num))
      else:
        logging.info('Verified number of calls to "DrawFrame".')
    if 'GgpIssueFrameToken' in children[i].window_text():
      found_ggp_issue_frame_token = True
      num = int(children[i + 1].window_text())
      if num < 30 or num > 3000:
        raise RuntimeError('Wrong number of calls to "GgpIssueFrameToken": ' +
                           str(num))
      else:
        logging.info('Verified number of calls to "GgpIssueFrameToken".')

  if not found_draw_frame:
    raise RuntimeError('Function DrawFrame was not found in the binary.')
  if not found_ggp_issue_frame_token:
    raise RuntimeError(
        'Function GgpIssueFrameToken was not found in the binary.')

  main_wnd.CloseButton.click_input()
  logging.info('Closed Orbit.')


if __name__ == '__main__':
  app.run(main)
