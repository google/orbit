# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(WebEngine)
add_library(WebEngine STATIC)

target_compile_options(WebEngine PRIVATE ${STRICT_COMPILE_FLAGS})
target_include_directories(WebEngine PUBLIC include/
                                     PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
  WebEngine PUBLIC OrbitBase Qt5::Widgets Qt5::WebEngine Qt5::WebEngineWidgets
                   Qt5::WebSockets Qt5::WebChannel)

target_sources(WebEngine PUBLIC include/WebEngine/Dialog.h
                                include/WebEngine/DeleteLaterDeleter.h
                                include/WebEngine/View.h)

target_sources(WebEngine PRIVATE Dialog.cpp JsonTransport.cpp JsonTransport.h View.cpp)
set_target_properties(WebEngine PROPERTIES AUTOMOC ON)
