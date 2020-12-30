"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from pywinauto.base_wrapper import BaseWrapper
from pywinauto.controls.uiawrapper import UIAWrapper, UIAElementInfo

from core.orbit_e2e import find_control


class Track:
    def __init__(self, control: BaseWrapper):
        self._container = control
        self._title = find_control(control, 'TabItem')
        self._content = find_control(control, 'Group')

    container = property(lambda self: self._container)
    title = property(lambda self: self._title)
    content = property(lambda self: self._content)
