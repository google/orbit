# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitGl CXX)
add_library(OrbitGl STATIC)

target_compile_options(OrbitGl PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  OrbitGl
  PUBLIC AccessibleCaptureViewElement.h
         AccessibleInterfaceProvider.h
         AccessibleThreadBar.h
         AccessibleTimeGraph.h
         AccessibleTrack.h
         AccessibleTriangleToggle.h
         AnnotationTrack.h
         App.h
         AsyncTrack.h
         Batcher.h
         BlockChain.h
         CallstackDataView.h
         CallstackThreadBar.h
         CallTreeView.h
         CaptureStats.h
         CaptureViewElement.h
         CaptureWindow.h
         CoreMath.h
         FramePointerValidatorClient.h
         FrameTrack.h
         FrameTrackOnlineProcessor.h
         Geometry.h
         GlCanvas.h
         GlSlider.h
         GlUtils.h
         GpuDebugMarkerTrack.h
         GpuSubmissionTrack.h
         GpuTrack.h
         GraphTrack.h
         Images.h
         ImGuiOrbit.h
         IntrospectionWindow.h
         LiveFunctionsController.h
         LiveFunctionsDataView.h
         ManualInstrumentationManager.h
         MemoryTrack.h
         ModulesDataView.h
         MultivariateTimeSeries.h
         PickingManager.h
         SamplingReport.h
         SamplingReportDataView.h
         SchedulerTrack.h
         SchedulingStats.h
         ScopeTree.h
         ShortenStringWithEllipsis.h
         StatusListener.h
         TextBox.h
         TextRenderer.h
         ThreadBar.h
         ThreadStateBar.h
         ThreadTrack.h
         TimeGraph.h
         TimeGraphLayout.h
         Timer.h
         TimerChain.h
         TimerInfosIterator.h
         TimerTrack.h
         Track.h
         TrackManager.h
         TriangleToggle.h
         TracepointsDataView.h
         TracepointThreadBar.h
         VariableTrack.h
         Viewport.h)

target_sources(
  OrbitGl
  PRIVATE AccessibleCaptureViewElement.cpp
          AccessibleInterfaceProvider.cpp
          AccessibleThreadBar.cpp
          AccessibleTimeGraph.cpp
          AccessibleTrack.cpp
          AccessibleTriangleToggle.cpp
          AnnotationTrack.cpp
          App.cpp
          AsyncTrack.cpp
          Batcher.cpp
          CallstackDataView.cpp
          CallstackThreadBar.cpp
          CallTreeView.cpp
          CaptureStats.cpp
          CaptureViewElement.cpp
          CaptureWindow.cpp
          CompareAscendingOrDescending.h
          DataManager.cpp
          FramePointerValidatorClient.cpp
          FrameTrack.cpp
          FrameTrackOnlineProcessor.cpp
          LiveFunctionsController.cpp
          GlCanvas.cpp
          GlSlider.cpp
          GlUtils.cpp
          GpuDebugMarkerTrack.cpp
          GpuSubmissionTrack.cpp
          GpuTrack.cpp
          GraphTrack.cpp
          ImGuiOrbit.cpp
          IntrospectionWindow.cpp
          LiveFunctionsDataView.cpp
          ManualInstrumentationManager.cpp
          MemoryTrack.cpp
          ModulesDataView.cpp
          PickingManager.cpp
          SamplingReport.cpp
          SamplingReportDataView.cpp
          SchedulerTrack.cpp
          SchedulingStats.cpp
          StringManager.cpp
          StringManager.h
          TextRenderer.cpp
          TimeGraph.cpp
          TimeGraphLayout.cpp
          TimerChain.cpp
          TimerInfosIterator.cpp
          TimerTrack.cpp
          ThreadBar.cpp
          ThreadStateBar.cpp
          ThreadTrack.cpp
          Track.cpp
          TrackManager.cpp
          TriangleToggle.cpp
          TracepointsDataView.cpp
          TracepointThreadBar.cpp
          VariableTrack.cpp
          Viewport.cpp)

target_include_directories(OrbitGl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  OrbitGl
  PUBLIC CaptureClient
         CaptureFileInfo
         ClientData
         ClientModel
         ClientServices
         CodeReport
         DataViews
         DisplayFormats
         GrpcProtos
         Introspection
         MetricsUploader
         OrbitAccessibility
         OrbitPaths
         PresetFile
         Qt5::Gui
         Symbols
         CONAN_PKG::Outcome
         CONAN_PKG::freetype-gl
         CONAN_PKG::freetype
         CONAN_PKG::glad
         CONAN_PKG::imgui
         gte::gte)

if(TARGET OpenGL::GLX AND TARGET OpenGL::OpenGL)
  target_link_libraries(OrbitGl PUBLIC OpenGL::GLX)
elseif(TARGET OpenGL::GL)
  target_link_libraries(OrbitGl PUBLIC OpenGL::GL)
endif()

add_executable(OrbitGlTests)

target_compile_options(OrbitGlTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitGlTests PRIVATE
               PickingManagerTest.h
               UnitTestSlider.h)

target_sources(OrbitGlTests PRIVATE
               BatcherTest.cpp
               BlockChainTest.cpp
               CaptureStatsTest.cpp
               CaptureWindowTest.cpp
               ClientFlags.cpp
               GlUtilsTest.cpp
               GpuTrackTest.cpp
               MultivariateTimeSeriesTest.cpp
               PickingManagerTest.cpp
               ScopedStatusTest.cpp
               ScopeTreeTest.cpp
               SliderTest.cpp
               ShortenStringWithEllipsisTest.cpp
               StringManagerTest.cpp
               TimerInfosIteratorTest.cpp
               ViewportTest.cpp)

target_link_libraries(
  OrbitGlTests
  PRIVATE OrbitGl
          GTest::Main)

register_test(OrbitGlTests)
