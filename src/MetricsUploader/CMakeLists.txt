# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(MetricsUploader)

add_library(MetricsUploader STATIC)

target_compile_options(MetricsUploader PRIVATE ${STRICT_COMPILE_FLAGS})

target_compile_features(MetricsUploader PUBLIC cxx_std_17)

target_include_directories(MetricsUploader PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
                                           PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_sources(MetricsUploader PUBLIC include/MetricsUploader/MetricsUploader.h
                                      include/MetricsUploader/MetricsUploaderStub.h
                                      include/MetricsUploader/Result.h
                                      include/MetricsUploader/ScopedMetric.h
                               PRIVATE Result.cpp
                                       ScopedMetric.cpp)

if (WIN32)
target_sources(MetricsUploader PRIVATE MetricsUploaderWindows.cpp)
else()
target_sources(MetricsUploader PRIVATE MetricsUploaderLinux.cpp)
endif()

target_link_libraries(MetricsUploader PUBLIC OrbitBase
                                             OrbitVersion
                                             CONAN_PKG::abseil)

if (WIN32)
target_link_libraries(MetricsUploader PUBLIC rpcrt4.lib)
endif()

# generate protos for communication
protobuf_generate(TARGET MetricsUploader PROTOS proto/orbit_log_event.proto)
target_include_directories(MetricsUploader PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(MetricsUploader PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/proto)
target_link_libraries(MetricsUploader PUBLIC CONAN_PKG::protobuf)

if (WIN32)
# build .dll for tests
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(MetricsUploaderClientWithoutSendEvent SHARED TestClients/ClientWithoutSendEvent.cpp)
target_link_libraries(MetricsUploaderClientWithoutSendEvent PRIVATE MetricsUploader)
set_target_properties(MetricsUploaderClientWithoutSendEvent PROPERTIES FOLDER "MetricsUploaderTestClients")

add_library(MetricsUploaderClientWithoutSetup SHARED TestClients/ClientWithoutSetup.cpp)
target_link_libraries(MetricsUploaderClientWithoutSetup PRIVATE MetricsUploader)
set_target_properties(MetricsUploaderClientWithoutSetup PROPERTIES FOLDER "MetricsUploaderTestClients")

add_library(MetricsUploaderClientWithoutShutdown SHARED TestClients/ClientWithoutShutdown.cpp)
target_link_libraries(MetricsUploaderClientWithoutShutdown PRIVATE MetricsUploader)
set_target_properties(MetricsUploaderClientWithoutShutdown PROPERTIES FOLDER "MetricsUploaderTestClients")

add_library(MetricsUploaderCompleteClient SHARED TestClients/CompleteClient.cpp)
target_link_libraries(MetricsUploaderCompleteClient PRIVATE MetricsUploader)
set_target_properties(MetricsUploaderCompleteClient PROPERTIES FOLDER "MetricsUploaderTestClients")

add_library(MetricsUploaderSetupWithErrorClient SHARED TestClients/SetupWithErrorClient.cpp)
target_link_libraries(MetricsUploaderSetupWithErrorClient PRIVATE MetricsUploader)
set_target_properties(MetricsUploaderSetupWithErrorClient PROPERTIES FOLDER "MetricsUploaderTestClients")

endif()

# add tests
add_executable(MetricsUploaderTests)

target_compile_options(MetricsUploaderTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(MetricsUploaderTests PRIVATE ScopedMetricTest.cpp)

if (WIN32)
target_sources(MetricsUploaderTests PRIVATE MetricsUploaderWindowsTest.cpp)
endif()

target_link_libraries(MetricsUploaderTests PRIVATE MetricsUploader
                                                   GTest::GTest
                                                   GTest::Main)

register_test(MetricsUploaderTests)
