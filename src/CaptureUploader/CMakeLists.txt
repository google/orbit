# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(CaptureUploader)
add_library(CaptureUploader INTERFACE)

target_include_directories(CaptureUploader INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(CaptureUploader INTERFACE  
        include/CaptureUploader/UploadDataInterface.h)