# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitGl CXX)
add_library(OrbitGl STATIC)

target_sources(
  OrbitGl
  PUBLIC AccessibleCaptureViewElement.h
         AccessibleInterfaceProvider.h
         AccessibleThreadBar.h
         AccessibleTimeGraph.h
         AccessibleTimeline.h
         AccessibleTrack.h
         AccessibleTrackContainer.h
         AccessibleTriangleToggle.h
         AnnotationTrack.h
         App.h
         AsyncTrack.h
         BasicPageFaultsTrack.h
         Batcher.h
         CallstackThreadBar.h
         CallTreeView.h
         CaptureStats.h
         CaptureViewElement.h
         CaptureWindow.h
         CGroupAndProcessMemoryTrack.h
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
         LineGraphTrack.h
         LiveFunctionsController.h
         ManualInstrumentationManager.h
         MajorPageFaultsTrack.h
         MemoryTrack.h
         MinorPageFaultsTrack.h
         MultivariateTimeSeries.h
         PageFaultsTrack.h
         PickingManager.h
         SamplingReport.h
         SchedulerTrack.h
         SchedulingStats.h
         ShortenStringWithEllipsis.h
         SimpleTimings.h
         StatusListener.h
         SystemMemoryTrack.h
         TextRenderer.h
         ThreadBar.h
         ThreadStateBar.h
         ThreadTrack.h
         TimeGraph.h
         TimeGraphLayout.h
         TimelineInfoInterface.h
         TimelineTicks.h
         TimelineUi.h
         Timer.h
         TimerInfosIterator.h
         TimerTrack.h
         TracepointThreadBar.h
         Track.h
         TrackContainer.h
         TrackManager.h
         TrackTestData.h
         TranslationStack.h
         TriangleToggle.h
         VariableTrack.h
         Viewport.h)

target_sources(
  OrbitGl
  PRIVATE AccessibleCaptureViewElement.cpp
          AccessibleInterfaceProvider.cpp
          AccessibleThreadBar.cpp
          AccessibleTimeGraph.cpp
          AccessibleTrack.cpp
          AccessibleTrackContainer.cpp
          AccessibleTriangleToggle.cpp
          AnnotationTrack.cpp
          App.cpp
          AsyncTrack.cpp
          BasicPageFaultsTrack.cpp
          Batcher.cpp
          CallstackThreadBar.cpp
          CallTreeView.cpp
          CaptureStats.cpp
          CaptureViewElement.cpp
          CaptureWindow.cpp
          CGroupAndProcessMemoryTrack.cpp
          FramePointerValidatorClient.cpp
          FrameTrack.cpp
          FrameTrackOnlineProcessor.cpp
          GlCanvas.cpp
          GlSlider.cpp
          GlUtils.cpp
          GpuDebugMarkerTrack.cpp
          GpuSubmissionTrack.cpp
          GpuTrack.cpp
          GraphTrack.cpp
          ImGuiOrbit.cpp
          IntrospectionWindow.cpp
          LineGraphTrack.cpp
          LiveFunctionsController.cpp
          ManualInstrumentationManager.cpp
          MajorPageFaultsTrack.cpp
          MemoryTrack.cpp
          MinorPageFaultsTrack.cpp
          PageFaultsTrack.cpp
          PickingManager.cpp
          SamplingReport.cpp
          SchedulerTrack.cpp
          SchedulingStats.cpp
          SimpleTimings.cpp
          SystemMemoryTrack.cpp
          TextRenderer.cpp
          TimeGraph.cpp
          TimeGraphLayout.cpp
          TimelineTicks.cpp
          TimelineUi.cpp
          TimerInfosIterator.cpp
          TimerTrack.cpp
          ThreadBar.cpp
          ThreadStateBar.cpp
          ThreadTrack.cpp
          TracepointThreadBar.cpp
          Track.cpp
          TrackContainer.cpp
          TrackManager.cpp
          TrackTestData.cpp
          TranslationStack.cpp
          TriangleToggle.cpp
          Viewport.cpp)

target_include_directories(OrbitGl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  OrbitGl
  PUBLIC CaptureClient
         CaptureFileInfo
         ClientData
         ClientFlags
         ClientModel
         ClientServices
         CodeReport
         Containers
         DataViews
         DisplayFormats
         GrpcProtos
         MetricsUploader
         OrbitAccessibility
         OrbitPaths
         PresetFile
         StringManager
         SymbolPaths
         Qt5::Gui
         Symbols
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

if(WIN32)
  target_compile_options(OrbitGl PRIVATE "$<$<CONFIG:Debug>:/bigobj>")

  target_link_libraries(
    OrbitGl
    PUBLIC WindowsUtils)
endif()


add_executable(OrbitGlTests)

target_sources(OrbitGlTests PRIVATE
               CaptureViewElementTester.h
               PickingManagerTest.h
               UnitTestSlider.h)

target_sources(OrbitGlTests PRIVATE
               BatcherTest.cpp
               CaptureStatsTest.cpp
               CaptureViewElementTest.cpp
               CaptureViewElementTester.cpp
               CaptureWindowTest.cpp
               GlUtilsTest.cpp
               GpuTrackTest.cpp
               MultivariateTimeSeriesTest.cpp
               PageFaultsTrackTest.cpp
               PickingManagerTest.cpp
               ScopedStatusTest.cpp
               SimpleTimingsTest.cpp
               SliderTest.cpp
               ShortenStringWithEllipsisTest.cpp
               TimelineTicksTest.cpp
               TimerInfosIteratorTest.cpp
               TranslationStackTest.cpp
               ThreadTrackTest.cpp
               TrackManagerTest.cpp
               ViewportTest.cpp)

target_link_libraries(
  OrbitGlTests
  PRIVATE OrbitGl
          GTest::Main)

register_test(OrbitGlTests)
