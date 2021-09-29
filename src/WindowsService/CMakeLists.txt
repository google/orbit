# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# To make OrbitAsioServer and OrbitGrpcServer available for fuzz-testing
# the target OrbitWindowsServiceLib was introduced. OrbitService still exists,
# but only builds main.cpp and links to OrbitWindowsServiceLib.
project(WindowsService)

add_library(WindowsServiceLib STATIC)
target_compile_options(WindowsServiceLib PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(WindowsServiceLib PRIVATE
        CaptureEventBuffer.h
        CaptureEventSender.h
        CaptureServiceImpl.cpp
        CaptureServiceImpl.h
        CaptureStartStopListener.h
        OrbitGrpcServer.cpp
        OrbitGrpcServer.h
        OrbitService.cpp
        OrbitService.h
        ProcessServiceImpl.cpp
        ProcessServiceImpl.h
        ProducerEventProcessor.h
        ProducerEventProcessor.cpp
        TracingHandler.cpp
        TracingHandler.h)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set_target_properties(WindowsServiceLib PROPERTIES COMPILE_FLAGS /wd4127)
endif()

target_include_directories(WindowsServiceLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(WindowsServiceLib PUBLIC
        GrpcProtos
        Introspection
        WindowsTracing
        WindowsUtils
        ObjectUtils
        OrbitVersion
        OrbitLib
        )

project(OrbitService)
add_executable(OrbitService main.cpp)
target_link_libraries(OrbitService PRIVATE WindowsServiceLib)
target_compile_options(OrbitService PRIVATE ${STRICT_COMPILE_FLAGS})

add_custom_command(TARGET OrbitService POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/OrbitLib/"
        $<TARGET_FILE_DIR:OrbitService>)

SET_TARGET_PROPERTIES(OrbitService PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:CONSOLE")

# add_executable(ServiceTests)
# target_compile_options(ServiceTests PRIVATE ${STRICT_COMPILE_FLAGS})

# target_sources(ServiceTests PRIVATE
#         ProcessListTest.cpp
#         ProcessTest.cpp
#         ProducerEventProcessorTest.cpp
#         ProducerSideServiceImplTest.cpp
#         ServiceUtilsTest.cpp)

# target_link_libraries(ServiceTests PRIVATE
#         WindowsServiceLib
#         GTest::Main)

# add_custom_command(TARGET ServiceTests POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/testdata
#         $<TARGET_FILE_DIR:ServiceTests>/testdata)

# register_test(ServiceTests PROPERTIES TIMEOUT 10)

# add_fuzzer(OrbitServiceUtilsFindSymbolsFilePathFuzzer
#            OrbitServiceUtilsFindSymbolsFilePathFuzzer.cpp)
# target_link_libraries(OrbitServiceUtilsFindSymbolsFilePathFuzzer PRIVATE WindowsServiceLib)
