# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(RemoteSymbolProvider)
add_library(RemoteSymbolProvider INTERFACE)

target_sources(RemoteSymbolProvider INTERFACE
    include/RemoteSymbolProvider/DownloadManagerInterface.h)

target_include_directories(RemoteSymbolProvider INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(RemoteSymbolProvider INTERFACE
    OrbitBase)