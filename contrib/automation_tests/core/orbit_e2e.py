"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import logging
from typing import Iterable, Callable
from copy import deepcopy
import time
import psutil

from absl import flags
from pywinauto import Application, timings
from pywinauto.base_wrapper import BaseWrapper
from pywinauto.findwindows import ElementNotFoundError
from pywinauto.keyboard import send_keys

flags.DEFINE_boolean(
    'dev_mode', False, 'Dev mode - expect Orbit MainWindow to be opened, '
    'and do not close Orbit at the end')


class OrbitE2EError(RuntimeError):
    pass


class E2ETestCase:
    """
    Encapsulates a single test executed as part of a E2E Test Suite.
    Inherit from this class to define a new test, and pass an instance of your test in the E2ETestSuite constructor
    to have it executed.

    All named arguments passed to the constructor of your test will be forwarded to the _execute method. To create
    a parameterized test, simply inherit from this class and provide your own _execute methods with any number of
    named arguments.
    """

    def __init__(self, **kwargs):
        self._suite = None
        self._args = kwargs

    suite = property(lambda self: self._suite)
    args = property(lambda self: deepcopy(self._args))

    def execute(self, suite: "E2ETestSuite"):
        self._suite = suite
        self._execute(**self._args)

    def expect_true(self, cond, description):
        if not cond:
            raise OrbitE2EError(
                'Error executing testcase %s, fragment %s. Condition expected to be True: "%s"' %
                (self.suite.name, self.__class__.__name__, description))

    def expect_eq(self, left, right, description):
        self.expect_true(left == right, description + " (got {}, expected {})".format(left, right))

    def find_control(self,
                     control_type=None,
                     name=None,
                     parent: BaseWrapper = None,
                     name_contains=None,
                     auto_id_leaf=None,
                     qt_class=None,
                     recurse=True,
                     raise_on_failure=True) -> BaseWrapper:
        """
        Returns the first child of BaseWrapper that matches all of the search parameters.
        As soon as a matching child is encountered, this function returns.

        If no child is found, an exception is thrown.
        """
        if not parent:
            parent = self.suite.top_window()
        return find_control(parent,
                            control_type=control_type,
                            name=name,
                            name_contains=name_contains,
                            auto_id_leaf=auto_id_leaf,
                            qt_class=qt_class,
                            recurse=recurse,
                            raise_on_failure=raise_on_failure)

    def find_context_menu_item(self, text: str, raise_on_failure=True):
        """
        Specialized version of find_control for context menu items. This is provided due to the special
        behavior of context menus: They are not parented underneath the current main window, but are treated
        as a separate window.
        """
        wait_for_condition(lambda: self.find_control(
            'MenuItem', text, parent=self.suite.application.top_window(), raise_on_failure=False) is
                           not None)
        return self.find_control('MenuItem',
                                 text,
                                 parent=self.suite.application.top_window(),
                                 raise_on_failure=raise_on_failure)

    def find_combo_box_item(self, text: str, raise_on_failure=True):
        """
        Specialized version of find_control for combo box items. This is provided due to the special
        behavior of expanded combo box lists: They are not parented underneath the current main window, but
        are treated as a separate window.
        """
        wait_for_condition(lambda: self.find_control(
            'ListItem', text, parent=self.suite.application.top_window(), raise_on_failure=False) is
                           not None)
        return self.find_control('ListItem',
                                 text,
                                 parent=self.suite.application.top_window(),
                                 raise_on_failure=raise_on_failure)

    def _execute(self, **kwargs):
        """
        Provide this method in your fragment
        """
        pass


class CloseOrbit(E2ETestCase):

    def _check_process_is_running(self, name: str):
        """
        Returns true if there is a running process containing name
        """
        for proc in psutil.process_iter():
            try:
                if name.lower() in proc.name().lower():
                    return True
            except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
                pass
        return False

    def _execute(self):
        # The sending of Alt + F4 (see below) only works if OrbitMainWindow is in focus. The next line checks
        # that no other dialog, warning or error is open
        wait_for_condition(
            lambda: self.find_control(control_type="Window", raise_on_failure=False) is None,
            max_seconds=30)

        # For some reason the line below does NOT work when Orbit is maximized - this is actually consistent with
        # the results of AccessibilityInsights as the close button seems to have no on-screen rect...
        # self.find_control("Button", "Close").click_input()
        # ... so just send Alt + F4
        send_keys('%{F4}')
        # Wait for the Orbit process to close.
        while self._check_process_is_running('Orbit'):
            time.sleep(1)
        logging.info('Closed Orbit.')


class E2ETestSuite:

    def __init__(self, test_name: str, test_cases: Iterable[E2ETestCase], dev_mode: bool = False):
        logging.info('E2E Test Suite "%s" started.', test_name)
        self._top_window = None
        self._application = None
        self._wait_for_orbit()
        self._test_name = test_name
        self._dev_mode = dev_mode or flags.FLAGS.dev_mode
        self._test_cases = test_cases[:]
        self._shared_data = {}

    name = property(lambda self: self._test_name)
    application = property(lambda self: self._application)
    dev_mode = property(lambda self: self._dev_mode)
    """
    Used to store data shared across test cases of the same suite. Can be used to e.g. persist timings or results
    from previously executed test cases within the same suite.
    """
    shared_data = property(lambda self: self._shared_data)

    def top_window(self, force_update=False):
        if force_update or not self._top_window:
            self._top_window = self._application.top_window()
        return self._top_window

    def set_up(self):
        timings.Timings.after_click_wait = 1.0
        timings.Timings.after_clickinput_wait = 1.0
        logging.info("Setting up with dev_mode = %s", self.dev_mode)
        self.top_window(True).set_focus()

    def tear_down(self):
        if not self._dev_mode:
            CloseOrbit().execute(self)
        else:
            logging.info("DEV MODE: Skipped closing Orbit")
        logging.info('Testcase "%s" executed without errors', self._test_name)
        timings.Timings.defaults()

    def execute(self):
        self.set_up()
        for test in self._test_cases:
            logging.info('Running test "%s (%s)"', test.__class__.__name__,
                         ", ".join("%s=%s" % (k, v) for k, v in test.args.items()))
            test.execute(self)
        self.tear_down()

    def _wait_for_orbit(self):
        logging.info('Waiting for Orbit')
        logging.info('Start waiting for Orbit.')

        def create_application():
            try:
                self._application = Application(backend='uia').connect(title_re='Orbit Profiler')
                return True
            except ElementNotFoundError:
                logging.error("Could not find Orbit window, retrying...")
                return False

        wait_for_condition(create_application, 600)
        logging.info('Connected to Orbit.')
        self.top_window(True).set_focus()


def wait_for_condition(any_callable: Callable,
                       max_seconds: int = 5,
                       interval: int = 1,
                       raise_exceptions: bool = False):
    """
    Wait until a condition is satisfied.

    :param any_callable: Any callable that evaluates to True or False. If this returns True, the condition is considered
        satisfied.
    :param max_seconds: Maximum time to wait for the condition to be satisfied
    :param interval: Sleep time in between calls to callable, in seconds
    :param raise_exceptions: If true, exceptions are raised and cause the wait to fail. Usually should only be used
        for debugging.
    """
    start = time.time()
    while time.time() - start < max_seconds:
        try:
            if any_callable():
                return
        except:
            if raise_exceptions:
                raise
            else:
                pass
        time.sleep(interval)
    raise OrbitE2EError('Wait time exceeded')


def find_control(parent: BaseWrapper,
                 control_type=None,
                 name=None,
                 name_contains=None,
                 auto_id_leaf=None,
                 qt_class=None,
                 recurse=True,
                 raise_on_failure=True) -> BaseWrapper or None:
    """
    Returns the first child of BaseWrapper that matches all of the search parameters.
    As soon as a matching child is encountered, this function returns.

    If no child is found, an exception is thrown.
    """
    if not recurse:
        desc = parent.children(control_type=control_type)
    else:
        desc = parent.descendants(control_type=control_type)

    for elem in desc:
        # This seems to be a lot more reliable than the properties exposed by the wrapper element
        # At least, this seems to consistently return the accessible name if it exists...
        elem_name = elem.element_info.element.CurrentName
        elem_text = elem.texts()[0] if elem.texts() else ""
        if (not qt_class or elem.class_name() == qt_class) and \
                (not name or elem_text == name or elem_name == name) and \
                (not name_contains or name_contains in elem_text or name_contains in elem_name) and \
                (not auto_id_leaf or elem.automation_id().rsplit(".", 1)[-1]):
            return elem

    if raise_on_failure:
        raise OrbitE2EError(
            'Could not find element of type %s (name="%s", name_contains="%s", qt_class="%s", recurse=%s).'
            % (control_type, name, name_contains, qt_class, recurse))
    else:
        return None
