"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

"""Inspect the top-down view in Orbit using pywinauto.

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
 - verify that the top-down view contains at least 3 rows
 - verify that the first item is "hello_* (all threads)"
 - verify that the second item is "hello_*]" or "Ggp*]"
 - verify that the children of the first item are "*clone" and "_start" (in any order)
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
  main_wnd.child_window(title="Top-Down").click_input()
  logging.info('Switched to Top-Down tab')
  
  # Now that the "Top-Down" tab is selected,
  # main_wnd.TreeView is the QTreeView of the top-down view.
  # main_wnd.TreeView.children(control_type='TreeItem') returns
  # every cell in the top-down view, in order by row and then column.
  # It can take a few seconds.
  logging.info('Listing items of the top-down view...')
  tree_items = main_wnd.TreeView.children(control_type='TreeItem')
  TOP_DOWN_ROW_CELL_COUNT = 6
  row_count_before_expansion = len(tree_items) / TOP_DOWN_ROW_CELL_COUNT
  
  if row_count_before_expansion < 3:
    raise RuntimeError('Less than 3 rows in the top-down view')

  if (not tree_items[0].window_text().startswith('hello_') or
      not tree_items[0].window_text().endswith(' (all threads)')):
    raise RuntimeError('First item of the top-down view is not "hello_* (all threads)"')
  logging.info('Verified that first item is "hello_* (all threads)"')

  if ((not tree_items[TOP_DOWN_ROW_CELL_COUNT].window_text().startswith('hello_') and
       not tree_items[TOP_DOWN_ROW_CELL_COUNT].window_text().startswith('Ggp')) or
      not tree_items[TOP_DOWN_ROW_CELL_COUNT].window_text().endswith(']')):
    raise RuntimeError('Second item of the top-down view is not "hello_*]" nor "Ggp*]"')
  logging.info('Verified that second item is "hello_*]" or "Ggp*]"')
  
  tree_items[0].double_click_input()
  logging.info('Expanded the first item')
  
  logging.info('Re-listing items of the top-down view...')
  tree_items = main_wnd.TreeView.children(control_type='TreeItem')
  row_count_after_expansion = len(tree_items) / TOP_DOWN_ROW_CELL_COUNT

  if row_count_after_expansion != row_count_before_expansion + 2:
    raise RuntimeError('First item of the top-down view doesn\'t have exactly two children')
  if (not ((tree_items[TOP_DOWN_ROW_CELL_COUNT].window_text().endswith('clone') and 
            tree_items[2 * TOP_DOWN_ROW_CELL_COUNT].window_text() == '_start') or 
           (tree_items[TOP_DOWN_ROW_CELL_COUNT].window_text() == '_start') and
            tree_items[2 * TOP_DOWN_ROW_CELL_COUNT].window_text().endswith('clone'))):
    raise RuntimeError('Children of the first item of the top-down view '
                       'are not "*clone" and "_start"')
  logging.info('Verified that children of the first item are "*clone" and "_start"')

  main_wnd.CloseButton.click_input()
  logging.info('Closed Orbit.')


if __name__ == '__main__':
  app.run(main)
