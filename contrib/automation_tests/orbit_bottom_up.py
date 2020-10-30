"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

"""Inspect the bottom-up view in Orbit using pywinauto.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test it needs
to by run from 64 bit python.

This automation script covers a basic workflow:
 - start Orbit
 - connect to a gamelet
 - select a process
 - take a capture
 - verify that the bottom-up view contains at least 10 rows
 - verify that the first item is "ioctl"
 - verify that the first child of the first item starts with "drm"
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
  orbit_testing.FocusOnCaptureWindow(application)
  orbit_testing.Capture(application, 5);
  
  main_wnd = application.window(title_re='orbitprofiler', found_index=0)
  main_wnd.child_window(title="Bottom-Up").click_input()
  logging.info('Switched to Bottom-Up tab')
  
  # Now that the "Bottom-Up" tab is selected,
  # main_wnd.TreeView is the QTreeView of the bottom-up view.
  # main_wnd.TreeView.children(control_type='TreeItem') returns
  # every cell in the bottom-up view, in order by row and then column.
  # It can take a few seconds.
  logging.info('Listing items of the bottom-up view...')
  tree_items = main_wnd.TreeView.children(control_type='TreeItem')
  BOTTOM_UP_ROW_CELL_COUNT = 5
  row_count_before_expansion = len(tree_items) / BOTTOM_UP_ROW_CELL_COUNT
  
  if row_count_before_expansion < 10:
    raise RuntimeError('Less than 10 rows in the bottom-up view')

  if tree_items[0].window_text() != 'ioctl':
    raise RuntimeError('First item of the bottom-up view is not "ioctl"')
  logging.info('Verified that first item is "ioctl"')

  tree_items[0].double_click_input()
  logging.info('Expanded the first item')

  logging.info('Re-listing items of the bottom-up view...')
  tree_items = main_wnd.TreeView.children(control_type='TreeItem')
  row_count_after_expansion = len(tree_items) / BOTTOM_UP_ROW_CELL_COUNT
  if row_count_after_expansion <= row_count_before_expansion:
    raise RuntimeError('First item of the bottom-up view has no children')
  
  if not tree_items[BOTTOM_UP_ROW_CELL_COUNT].window_text().startswith('drm'):
    raise RuntimeError('First child of the first item ("ioctl") '
                       'of the bottom-up view doesn\'t start with "drm"')
  logging.info('Verified that first child of the first item starts with "drm"')

  main_wnd.CloseButton.click_input()
  logging.info('Closed Orbit.')


if __name__ == '__main__':
  app.run(main)
