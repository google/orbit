# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(ApiUtils STATIC)

target_sources(ApiUtils PUBLIC
        include/ApiUtils/ApiEnableInfo.h
        include/ApiUtils/Event.h
        include/ApiUtils/EncodedString.h
        include/ApiUtils/GetFunctionTableAddressPrefix.h)

target_sources(ApiUtils PRIVATE
        EncodedString.cpp
        Event.cpp)

target_link_libraries(ApiUtils PUBLIC
        ApiInterface
        GrpcProtos
        OrbitBase)

target_include_directories(ApiUtils PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

add_executable(ApiUtilsTests)

target_sources(ApiUtilsTests PRIVATE
        EncodedStringTest.cpp)

target_link_libraries(ApiUtilsTests PRIVATE
        ApiUtils
        GTest_Main)

register_test(ApiUtilsTests)
