"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging
import time
from pywinauto.application import Application

def WaitForOrbit():
  """Waits for Orbit to start up."""
  while True:
    try:
      Application(backend='uia').connect(title_re='orbitprofiler')
      break
    except pywinauto.findwindows.ElementNotFoundError:
      pass

def ConnectToGamelet(application):
  """Connect to the first gamelet in the start dialog.

  Args:
     application: pywinauto application object for Orbit.
  """
  # Connect to the first (and only) gamelet.
  dialog = application.window(title_re='orbitprofiler')
  dialog.set_focus()
  # DataItem0 contains the name of the first gamelet.
  dialog.DataItem0.wait('exists', timeout=100)
  dialog.DataItem0.click_input()
  dialog.OK.click_input()
  logging.info('Connected to Gamelet')
  # Wait until the service is deployed and the main window is opened.
  # Waiting for the window does not really work since there is a progress dialog
  # with an identical title popping up in between. So we just sleep.
  time.sleep(15)

def SelectProcess(application, process_search_term):
  """Types 'process_search_term' into the process search bar and then selects the first
     process in the tree view. This allows selecting the game process by using an 
     appropriate search term that filters out all other processes.
  Args:
     application: pywinauto application object for Orbit.
     process_search_term: The term to search for among processes.
  """
  main_wnd = application.window(title_re='orbitprofiler', found_index=0)
  time.sleep(2)
  main_wnd.ProcessesEdit.type_keys(process_search_term)
  main_wnd.ProcessesTreeView.DataItem0.wait('exists', timeout=100)
  main_wnd.ProcessesTreeView.DataItem0.click_input()
  logging.info('Selected process based on search: %s', process_search_term)
  time.sleep(5)

def LoadSymbols(application, module_search_term):
  """Types 'module_search_term' in the modules search bar and then selects the first
     module in the modules tree view. The allows selecting a modules by using an 
     appropriate search term. 
  Args:
     application: pywinauto application object for Orbit.
     module_search_term: The term to search for among modules.
  """
  main_wnd = application.window(title_re='orbitprofiler', found_index=0)
  time.sleep(2)
  main_wnd.ModulesEdit.type_keys(module_search_term)
  main_wnd.ModulesTreeView.DataItem0.wait('exists', timeout=100)
  main_wnd.ModulesTreeView.DataItem0.click_input(button='right')
  main_wnd.LoadSymbols.click_input()
  logging.info('Loaded symbols for module based on search: %s', module_search_term)
  time.sleep(5)

def HookFunction(application, function_search_term):
  """Types 'function_search_term' in the function search bar and then hooks the first
     function in the functions tree view. The allows hooking a single function using an
     appropriate search term. 
  Args:
     application: pywinauto application object for Orbit.
     function_search_term: The term to search for among function.
  """
  main_wnd = application.window(title_re='orbitprofiler', found_index=0)
  time.sleep(2)
  # There is no properly named control identifier for the function controls, this is
  # the right one based on trial and error.
  main_wnd.Edit1.type_keys(function_search_term)
  main_wnd.TreeView1.DataItem0.wait('exists', timeout=100)
  main_wnd.TreeView1.DataItem0.click_input(button='right')
  main_wnd.Hook.click_input()
  logging.info('Hooked function based on search: %s', function_search_term)
  time.sleep(5)

def FocusOnCaptureWindow(application):
  """Focuses on the capture window

  Args:
     application: pywinauto application object for Orbit.
  """
  main_wnd = application.window(title_re='orbitprofiler', found_index=0)

  # The capture tab can not be found in the usual way so the focus is set by
  # clicking on the widget. The x-coordinate for the click is obtained from the
  # mid_point of the capture tab header, the y-coordinate is taken from the
  # mid_point of the functions tab on the right.
  # Obtaining the coordinates is done before clicking on the tab since
  # pywinauto becomes painfully slow once the tab is visible.
  click_x = main_wnd.captureTabItem.rectangle().mid_point().x
  click_y = main_wnd.SessionsTreeView.rectangle().mid_point().y

  # Change into capture tab.
  main_wnd.captureTabItem.click_input()
  logging.info('Changed into capture view')

  # Focus the capture tab and take a five second capture.
  main_wnd.click_input(coords=(click_x, click_y))

def Capture(application, length):
  """Takes a capture of given length.

     Assumes that the capture window is in focus!
  Args:
     application: pywinauto application object for Orbit.
  """
  main_wnd = application.window(title_re='orbitprofiler', found_index=0)
  main_wnd.type_keys('X')
  time.sleep(length)
  main_wnd.type_keys('X')
  logging.info('Captured for %d seconds', length)
