# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitQt CXX)

add_executable(OrbitQt)

target_compile_options(OrbitQt PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  OrbitQt
  PRIVATE AccessibilityAdapter.h
          CallTreeViewItemModel.h
          CallTreeWidget.h
          CaptureOptionsDialog.h
          Connections.h
          ConnectToStadiaWidget.h
          CopyKeySequenceEnabledTreeView.h
          CutoutWidget.h
          DeploymentConfigurations.h
          ElidedLabel.h
          Error.h
          EventLoop.h
          FilterPanelWidget.h
          FilterPanelWidgetAction.h
          orbitaboutdialog.h
          orbitdataviewpanel.h
          orbiteventiterator.h
          orbitglwidget.h
          orbitlivefunctions.h
          orbitmainwindow.h
          orbitsamplingreport.h
          orbittablemodel.h
          orbittreeview.h
          opengldetect.h
          ProcessItemModel.h
          OverlayWidget.h
          ProfilingTargetDialog.h
          resource.h
          servicedeploymanager.h
          TargetConfiguration.h
          TutorialContent.h
          TutorialOverlay.h)

target_sources(
  OrbitQt
  PRIVATE AccessibilityAdapter.cpp
          CallTreeViewItemModel.cpp
          CallTreeWidget.cpp
          CallTreeWidget.ui
          CaptureOptionsDialog.cpp
          CaptureOptionsDialog.ui
          ConnectToStadiaWidget.cpp
          ConnectToStadiaWidget.ui
          Error.cpp
          ElidedLabel.cpp
          FilterPanelWidget.cpp
          FilterPanelWidgetAction.cpp
          StatusListenerImpl.cpp
          DeploymentConfigurations.cpp
          main.cpp
          orbitaboutdialog.cpp
          orbitaboutdialog.ui
          orbitdataviewpanel.cpp
          orbitdataviewpanel.ui
          orbiteventiterator.cpp
          orbiteventiterator.ui
          orbitglwidget.cpp
          orbitlivefunctions.cpp
          orbitlivefunctions.ui
          orbitmainwindow.cpp
          orbitmainwindow.ui
          orbitsamplingreport.cpp
          orbitsamplingreport.ui
          orbittablemodel.cpp
          orbittreeview.cpp
          opengldetect.cpp
          ProcessItemModel.cpp
          OverlayWidget.cpp
          OverlayWidget.ui
          ProfilingTargetDialog.cpp
          ProfilingTargetDialog.ui
          servicedeploymanager.cpp
          TutorialContent.cpp
          TutorialOverlay.cpp
          TutorialOverlay.ui
          ../../icons/orbiticons.qrc
          ../../images/orbitimages.qrc
          # Todo (176066030): OrbitGl relies on flags usually defined in the OrbitQt executable, so the
          # tests needs this to link against them. To avoid multiple definitions, we compile the flags
          # cpp with the tests
          ../OrbitGl/ClientFlags.cpp)

target_include_directories(OrbitQt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  OrbitQt
  PRIVATE CodeViewer
          MetricsUploader
          OrbitAccessibility
          OrbitCore
          OrbitGl
          OrbitGgp
          OrbitSshQt
          OrbitVersion
          QtUtils
          SourcePathsMapping
          Qt5::Widgets
          Qt5::Core
          SyntaxHighlighter
          qtpropertybrowser::qtpropertybrowser)

if(WITH_CRASH_HANDLING)
  target_link_libraries(OrbitQt PRIVATE CrashHandler)
  target_compile_definitions(OrbitQt PRIVATE ORBIT_CRASH_HANDLING)
endif()

if(WIN32)
  set_target_properties(OrbitQt PROPERTIES WIN32_EXECUTABLE ON)

  include(${CMAKE_SOURCE_DIR}/cmake/version.cmake)
  GenerateVersionFile("${CMAKE_CURRENT_BINARY_DIR}/OrbitQt.rc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/OrbitQt.rc.in" OrbitQt)
  target_sources(OrbitQt PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/OrbitQt.rc)

  get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
  find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

  add_custom_command(
    TARGET OrbitQt
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E env PATH="${_qt_bin_dir}"
            "${WINDEPLOYQT_EXECUTABLE}" --pdb "$<TARGET_FILE:OrbitQt>"
    COMMENT "Running windeployqt...")
endif()

set_target_properties(OrbitQt PROPERTIES OUTPUT_NAME "Orbit")
set_target_properties(OrbitQt PROPERTIES AUTOMOC ON)
set_target_properties(OrbitQt PROPERTIES AUTOUIC ON)
set_target_properties(OrbitQt PROPERTIES AUTORCC ON)
iwyu_add_dependency(OrbitQt)
strip_symbols(OrbitQt)

add_executable(OrbitQtTests)

target_compile_options(OrbitQtTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitQtTests
        PRIVATE AccessibilityAdapter.h
                CutoutWidget.h
                EventLoop.h
                TutorialOverlay.h)
target_sources(OrbitQtTests
        PRIVATE AccessibilityAdapter.cpp
                AccessibilityAdapterTest.cpp
                EventLoopTest.cpp
                ProcessItemModelTest.cpp
                ProcessItemModel.cpp
                StatusListenerImplTest.cpp
                StatusListenerImpl.cpp
                TutorialOverlayTest.cpp
                TutorialOverlay.cpp
                TutorialOverlay.ui
                ../OrbitGl/ClientFlags.cpp
                ../../images/orbitimages.qrc)

target_include_directories(OrbitQtTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  OrbitQtTests
  PRIVATE OrbitAccessibility
          OrbitCore
          OrbitGl
          QtUtils
          Qt5::Widgets
          Qt5::Core
          Qt5::Test
          GTest::QtGuiMain)

set_target_properties(OrbitQtTests PROPERTIES AUTOMOC ON)
set_target_properties(OrbitQtTests PROPERTIES AUTOUIC ON)
set_target_properties(OrbitQtTests PROPERTIES AUTORCC ON)
iwyu_add_dependency(OrbitQtTests)

if(WIN32)
  get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
  find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

  add_custom_command(
    TARGET OrbitQtTests
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E env PATH="${_qt_bin_dir}"
            "${WINDEPLOYQT_EXECUTABLE}" --pdb "$<TARGET_FILE:OrbitQtTests>"
    COMMENT "Running windeployqt...")
endif()


set(test_properties "")

# QT_QPA_PLATFORM=offscreen is currently not supported under Windows.
# So if requested we disable the test all the way.
# This is mainly a work-around for the CI, especially the presubmit checks.
if (WIN32 AND "$ENV{QT_QPA_PLATFORM}" STREQUAL "offscreen")
  message(STATUS "Disabling OrbitQt-tests since they don't work in a headless setup")
  list(APPEND test_properties DISABLED TRUE)
endif()

if (NOT WIN32)
  # On Linux we can always run the tests with offscreen rendering. That won't do any harm
  # and it avoid test failures in headless environments like SSH-sessions.
  list(APPEND test_properties ENVIRONMENT QT_QPA_PLATFORM=offscreen)
endif()

register_test(OrbitQtTests PROPERTIES ${test_properties})
