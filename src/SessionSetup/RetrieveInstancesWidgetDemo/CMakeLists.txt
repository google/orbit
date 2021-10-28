# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(RetrieveInstancesWidgetDemo)
add_executable(RetrieveInstancesWidgetDemo main.cpp)

target_link_libraries(RetrieveInstancesWidgetDemo PRIVATE SessionSetup
                                                  Qt5::Gui)
