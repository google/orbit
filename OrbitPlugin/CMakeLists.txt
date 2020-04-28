# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(OrbitPlugin STATIC)

target_sources(OrbitPlugin PRIVATE OrbitData.h OrbitSDK.h OrbitUserData.h
                                   UserPlugin.h)
target_sources(OrbitPlugin PUBLIC UserPlugin.cpp)

target_link_libraries(OrbitPlugin PUBLIC OrbitCore imgui::imgui)
