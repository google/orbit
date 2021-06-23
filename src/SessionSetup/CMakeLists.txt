# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(SessionSetup)
add_library(SessionSetup STATIC)

target_compile_options(SessionSetup PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  SessionSetup
  PUBLIC  include/SessionSetup/Connections.h
          include/SessionSetup/ConnectToStadiaWidget.h
          include/SessionSetup/DeploymentConfigurations.h
          include/SessionSetup/DoubleClickableLabel.h
          include/SessionSetup/Error.h
          include/SessionSetup/OverlayWidget.h
          include/SessionSetup/ProcessItemModel.h
          include/SessionSetup/ServiceDeployManager.h
          include/SessionSetup/SessionSetupDialog.h
          include/SessionSetup/TargetConfiguration.h
          include/SessionSetup/TargetLabel.h)

target_include_directories(SessionSetup PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(
  SessionSetup
  PRIVATE ConnectToStadiaWidget.cpp
          ConnectToStadiaWidget.ui
          DeploymentConfigurations.cpp
          Error.cpp
          OverlayWidget.cpp
          OverlayWidget.ui
          ProcessItemModel.cpp
          ServiceDeployManager.cpp
          SessionSetupDialog.cpp
          SessionSetupDialog.ui
          TargetLabel.cpp
          TargetLabel.ui
          ../../icons/orbiticons.qrc)

target_link_libraries(
  SessionSetup
  PUBLIC  CaptureFileInfo
          ClientData
          ClientServices
          MetricsUploader
          OrbitBase
          OrbitGgp
          OrbitSsh
          OrbitSshQt
          QtUtils
          CONAN_PKG::abseil
          CONAN_PKG::grpc
          Qt5::Widgets)

set_target_properties(SessionSetup PROPERTIES AUTOMOC ON)
set_target_properties(SessionSetup PROPERTIES AUTOUIC ON)

add_executable(SessionSetupTest)

target_compile_options(SessionSetupTest PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  SessionSetupTest
  PRIVATE DoubleClickableLabelTest.cpp
          ProcessItemModelTest.cpp
          SessionSetupDialogTest.cpp
          TargetLabelTest.cpp
          TestEnvironmentAbslFlags.cpp)


target_link_libraries(
  SessionSetupTest
  PRIVATE SessionSetup
          GTest::QtGuiMain
          Qt5::Test)

if (WIN32 AND "$ENV{QT_QPA_PLATFORM}" STREQUAL "offscreen")
  register_test(SessionSetupTest PROPERTIES DISABLED TRUE)
endif()

if (NOT WIN32)
  register_test(SessionSetupTest PROPERTIES ENVIRONMENT QT_QPA_PLATFORM=offscreen)
endif()
