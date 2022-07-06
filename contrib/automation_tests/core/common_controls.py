"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import time
from math import floor

from pywinauto import Application
from pywinauto.base_wrapper import BaseWrapper
from pywinauto.controls.uiawrapper import UIAWrapper, UIAElementInfo
from pywinauto import mouse

from core.orbit_e2e import find_control


class Track:

    def __init__(self, control: BaseWrapper):
        self._container = control
        self._name = control.texts()[0]
        self._title = find_control(control, 'TabItem')
        self._callstacks = find_control(control, 'Pane', 'Callstacks', raise_on_failure=False)
        self._thread_states = find_control(control, 'Pane', 'ThreadState', raise_on_failure=False)
        self._tracepoints = find_control(control, 'Pane', 'Tracepoints', raise_on_failure=False)
        self._timers = find_control(control, 'Pane', 'Timers', raise_on_failure=False)
        self._triangle_toggle = find_control(control,
                                             'Button',
                                             'TriangleToggle',
                                             raise_on_failure=False)

    container = property(lambda self: self._container)
    title = property(lambda self: self._title)
    callstacks = property(lambda self: self._callstacks)
    thread_states = property(lambda self: self._thread_states)
    tracepoints = property(lambda self: self._tracepoints)
    timers = property(lambda self: self._timers)
    triangle_toggle = property(lambda self: self._triangle_toggle)
    name = property(lambda self: self._name)


class Table:

    def __init__(self, control: BaseWrapper):
        self._table = control

    def get_row_count(self):
        return self._table.iface_grid.CurrentRowCount

    def get_column_count(self):
        return self._table.iface_grid.CurrentColumnCount

    def get_item_at(self, row: int, col: int) -> BaseWrapper:
        """
        This uses the low-level API to return the element at row x col.
        pywinauto does not seem to provide row/col access for TreeView, even though the element supports the grid pattern.
        Using get_item does not return items that are currently hidden. Unfortunately this also means that columns
        hidden due to scrolling are NOT handled correctly, and get_item(col + row * col_count) does not always yield
        the correct result...

        We really should not need to use the low-level API like this.
        """
        element_info = UIAElementInfo(self._table.iface_grid.GetItem(row, col))
        wrapper = UIAWrapper(element_info)
        return wrapper

    def find_first_item_row(self, text: str, column: int, partial_match=False) -> int or None:
        row_count = self.get_row_count()
        for i in range(row_count):
            item = self.get_item_at(i, column)
            item_text = item.texts()[0]
            if item_text == text or partial_match and text in item_text:
                return i

        return None


class DataViewPanel:

    def __init__(self, control: BaseWrapper):
        self._panel = control
        self._table = find_control(control, "Tree", "DataView")
        self._table_obj = Table(self._table)
        self._refresh_button = find_control(control, "Button", "Refresh", raise_on_failure=False)
        self._filter = find_control(control, "Edit", "Filter")

    panel = property(lambda self: self._panel)
    table = property(lambda self: self._table)
    refresh_button = property(lambda self: self._refresh_button)
    filter = property(lambda self: self._filter)

    def get_row_count(self):
        return self._table_obj.get_row_count()

    def get_column_count(self):
        return self._table_obj.get_column_count()

    def get_item_at(self, row: int, col: int) -> BaseWrapper:
        return self._table_obj.get_item_at(row, col)

    def find_first_item_row(self, text: str, column: int, partial_match=False) -> int:
        return self._table_obj.find_first_item_row(text, column, partial_match)


def get_tooltip(app: Application, control: BaseWrapper) -> str:
    old_window_handle = app.top_window().handle

    rectangle = control.rectangle()
    position = (rectangle.left + floor((rectangle.right - rectangle.left) / 2),
                rectangle.top + floor((rectangle.bottom - rectangle.top) / 2))

    control.set_focus()
    mouse.move(position)
    mouse.move((position[0] + 1, position[1]))
    time.sleep(2)

    tooltip = app.top_window()
    # If the top window didn't change, the control does not have a tooltip
    if old_window_handle != tooltip.handle:
        return tooltip.texts()[0] if tooltip.texts() else ""
    else:
        return ""
