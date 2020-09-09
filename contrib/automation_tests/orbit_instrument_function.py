"""Run a basic workflow in Orbit using pywinauto.

Before this script is run there needs to be a gamelet reserved and
"hello_ggp_standalone" has to be started.

The script requires absl and pywinauto. Since pywinauto requires the bitness of
the python installation to match the bitness of the program under test it needs
to by run from  64 bit python.

This automation script covers a basic workflow:
 - start Orbit
 - connects to a gamelet
 - select a process and loads debug symbols
 - instrument a function
 - take a capture and verify the hooked function is recorded
"""
import logging
import time
from absl import app
import pywinauto
from pywinauto.application import Application


def WaitForOrbit():
  while True:
    try:
      Application(backend='uia').connect(title_re='orbitprofiler')
      break
    except pywinauto.findwindows.ElementNotFoundError:
      pass


def main(argv):
  WaitForOrbit()
  application = Application(backend='uia').connect(title_re='orbitprofiler')

  if application.is_process_running():
    logging.info('Orbit is running.')
  else:
    logging.info('Orbit is not running.')

  # Connect to the first (and only) gamelet.
  dialog = application.window(title_re='orbitprofiler')
  dialog.set_focus()
  # DataItem0 contains the name of the first gamelet.
  dialog.DataItem0.click_input()
  dialog.OK.click_input()
  logging.info('Connected to Gamelet.')

  # Wait until the service is deployed and the main window is opened.
  # Waiting for the window does not really work since there is a progress dialog
  # with an identical title popping up in between. So we just sleep.
  time.sleep(15)

  # Choose hello_ggp_stanalone as the process to profile.
  main_wnd = application.window(title_re='orbitprofiler', found_index=0)
  main_wnd.ProcessesEdit.type_keys('hello_')
  # Wait for the game to appear in the ProcessesTreeView.
  main_wnd.ProcessesTreeView.hello_ggp_stand.wait('exists', timeout=100)
  main_wnd.ProcessesTreeView.hello_ggp_stand.click_input()
  logging.info('Selected process "hello_ggp_stand".')

  # Load debug symbols for hello_ggp_standalone module.
  time.sleep(2)
  main_wnd.ModulesTreeView.hello_ggp_standalone.click_input(button='right')
  main_wnd.LoadSymbols.click_input()
  logging.info('Loaded symbols for module "hello_ggp_standalone".')

  # Hook DrawFrame function.
  main_wnd.TreeView1.DrawFrame.click_input(button='right')
  main_wnd.Hook.click_input()
  logging.info('Hooked function "DrawFrame"')

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
  main_wnd.type_keys('X')
  time.sleep(5)
  main_wnd.type_keys('X')
  logging.info('Took a five second capture.')

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
