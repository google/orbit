# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(SessionSetup)
add_library(SessionSetup STATIC)

target_sources(
  SessionSetup
  PUBLIC  include/SessionSetup/Connections.h
          include/SessionSetup/ConnectToLocalWidget.h
          include/SessionSetup/ConnectToTargetDialog.h
          include/SessionSetup/DeploymentConfigurations.h
          include/SessionSetup/DoubleClickableLabel.h
          include/SessionSetup/Error.h
          include/SessionSetup/LoadCaptureWidget.h
          include/SessionSetup/OrbitServiceInstance.h
          include/SessionSetup/OverlayWidget.h
          include/SessionSetup/OtherUserDialog.h
          include/SessionSetup/ProcessItemModel.h
          include/SessionSetup/ProcessLauncherWidget.h
          include/SessionSetup/ProcessListWidget.h
          include/SessionSetup/ServiceDeployManager.h
          include/SessionSetup/SessionSetupDialog.h
          include/SessionSetup/SessionSetupUtils.h
          include/SessionSetup/TargetConfiguration.h
          include/SessionSetup/TargetLabel.h)

target_include_directories(SessionSetup PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(
  SessionSetup
  PRIVATE ConnectToLocalWidget.cpp
          ConnectToLocalWidget.ui
          ConnectToTargetDialog.cpp
          ConnectToTargetDialog.ui
          DeploymentConfigurations.cpp
          Error.cpp
          LoadCaptureWidget.cpp
          LoadCaptureWidget.ui
          OrbitServiceInstance.cpp
          OtherUserDialog.cpp
          OtherUserDialog.ui
          OverlayWidget.cpp
          OverlayWidget.ui
          ProcessItemModel.cpp
          ProcessLauncherWidget.cpp
          ProcessLauncherWidget.ui
          ProcessListWidget.cpp
          ProcessListWidget.ui
          ServiceDeployManager.cpp
          SessionSetupDialog.cpp
          SessionSetupDialog.ui
          SessionSetupUtils.cpp
          TargetLabel.cpp
          TargetLabel.ui
          ../../icons/orbiticons.qrc)

target_link_libraries(
  SessionSetup
  PUBLIC  CaptureFileInfo
          ClientData
          ClientFlags
          ClientServices
          OrbitBase
          OrbitSsh
          OrbitSshQt
          QtUtils
          absl::flags
          absl::flat_hash_map
          absl::str_format
          absl::strings
          grpc::grpc
          Qt5::Widgets)

set_target_properties(SessionSetup PROPERTIES AUTOMOC ON)
set_target_properties(SessionSetup PROPERTIES AUTOUIC ON)

add_executable(SessionSetupTests)

target_sources(
  SessionSetupTests
  PRIVATE ConnectToLocalWidgetTest.cpp
          DoubleClickableLabelTest.cpp
          LoadCaptureWidgetTest.cpp
          OrbitServiceInstanceTest.cpp
          OtherUserDialogTest.cpp
          ProcessItemModelTest.cpp
          ProcessListWidgetTest.cpp
          SessionSetupDialogTest.cpp
          SessionSetupUtilsTest.cpp
          TargetLabelTest.cpp)


target_link_libraries(
  SessionSetupTests
  PRIVATE SessionSetup
          TestUtils
          GTest::QtGuiMain
          Qt5::Test)

if (WIN32 AND "$ENV{QT_QPA_PLATFORM}" STREQUAL "offscreen")
  register_test(SessionSetupTests PROPERTIES DISABLED TRUE)
endif()

if (NOT WIN32)
  register_test(SessionSetupTests PROPERTIES ENVIRONMENT QT_QPA_PLATFORM=offscreen)
endif()