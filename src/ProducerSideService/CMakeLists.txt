# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(ProducerSideService)
add_library(ProducerSideService STATIC)

target_include_directories(ProducerSideService PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(ProducerSideService PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}) 

target_sources(ProducerSideService PUBLIC
        include/ProducerSideService/BuildAndStartProducerSideServer.h
        include/ProducerSideService/ProducerSideServer.h
        include/ProducerSideService/ProducerSideServiceImpl.h)    
        
target_sources(ProducerSideService PRIVATE
        BuildAndStartProducerSideServerWithUri.h
        ProducerSideServer.cpp
        ProducerSideServiceImpl.cpp)

if (WIN32)        
target_sources(ProducerSideService PRIVATE
        BuildAndStartProducerSideServerWindows.cpp)
else()
target_sources(ProducerSideService PRIVATE
        BuildAndStartProducerSideServerLinux.cpp)
endif()

target_link_libraries(ProducerSideService PUBLIC
        CaptureServiceBase
        GrpcProtos
        ProducerSideChannel)

add_executable(ProducerSideServiceTests)

target_sources(ProducerSideServiceTests PRIVATE
        ProducerSideServiceImplTest.cpp)

target_link_libraries(ProducerSideServiceTests PRIVATE
        ProducerEventProcessor
        ProducerSideService
        TestUtils
        GTest_Main)

register_test(ProducerSideServiceTests PROPERTIES TIMEOUT 10)
