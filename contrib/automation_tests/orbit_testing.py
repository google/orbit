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
  logging.info('Connected to Gamelet.')
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
  main_wnd.ProcessesEdit.type_keys(process_search_term)
  # Wait for the process to appear in the ProcessesTreeView.
  main_wnd.ProcessesTreeView.hello_ggp_stand.wait('exists', timeout=100)
  main_wnd.ProcessesTreeView.hello_ggp_stand.click_input()
  logging.info('Selected process')
  time.sleep(5)