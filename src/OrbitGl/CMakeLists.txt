# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitGl CXX)
add_library(OrbitGl STATIC)

target_compile_definitions(OrbitGl PUBLIC WIN32_LEAN_AND_MEAN)

target_sources(
  OrbitGl
  PUBLIC include/OrbitGl/AccessibleButton.h
         include/OrbitGl/AccessibleCaptureViewElement.h
         include/OrbitGl/AccessibleInterfaceProvider.h
         include/OrbitGl/AccessibleTimeGraph.h
         include/OrbitGl/AccessibleTrack.h
         include/OrbitGl/AccessibleTriangleToggle.h
         include/OrbitGl/AnnotationTrack.h
         include/OrbitGl/AsyncTrack.h
         include/OrbitGl/BasicPageFaultsTrack.h
         include/OrbitGl/Batcher.h
         include/OrbitGl/BatcherInterface.h
         include/OrbitGl/BatchRenderGroup.h
         include/OrbitGl/Button.h
         include/OrbitGl/CallstackThreadBar.h
         include/OrbitGl/CallTreeView.h
         include/OrbitGl/CaptureStats.h
         include/OrbitGl/CaptureViewElement.h
         include/OrbitGl/CaptureWindow.h
         include/OrbitGl/CGroupAndProcessMemoryTrack.h
         include/OrbitGl/CoreMath.h
         include/OrbitGl/FormatCallstackForTooltip.h
         include/OrbitGl/FrameTrack.h
         include/OrbitGl/FrameTrackOnlineProcessor.h
         include/OrbitGl/Geometry.h
         include/OrbitGl/GlCanvas.h
         include/OrbitGl/GlSlider.h
         include/OrbitGl/GlUtils.h
         include/OrbitGl/GpuDebugMarkerTrack.h
         include/OrbitGl/GpuSubmissionTrack.h
         include/OrbitGl/GpuTrack.h
         include/OrbitGl/GraphTrack.h
         include/OrbitGl/GraphTrackDataAggregator.h
         include/OrbitGl/IntrospectionWindow.h
         include/OrbitGl/LineGraphTrack.h
         include/OrbitGl/LiveFunctionsController.h
         include/OrbitGl/ManualInstrumentationManager.h
         include/OrbitGl/MajorPageFaultsTrack.h
         include/OrbitGl/MemoryTrack.h
         include/OrbitGl/MinorPageFaultsTrack.h
         include/OrbitGl/MockTimelineInfo.h
         include/OrbitGl/MultivariateTimeSeries.h
         include/OrbitGl/OpenGlBatcher.h
         include/OrbitGl/OrbitApp.h
         include/OrbitGl/QtTextRenderer.h
         include/OrbitGl/PageFaultsTrack.h
         include/OrbitGl/PickingManager.h
         include/OrbitGl/PrimitiveAssembler.h
         include/OrbitGl/SamplingReport.h
         include/OrbitGl/SchedulerTrack.h
         include/OrbitGl/SchedulingStats.h
         include/OrbitGl/SelectionData.h
         include/OrbitGl/ShortenStringWithEllipsis.h
         include/OrbitGl/SimpleTimings.h
         include/OrbitGl/StaticTimeGraphLayout.h
         include/OrbitGl/SymbolLoader.h
         include/OrbitGl/SystemMemoryTrack.h
         include/OrbitGl/TextRenderer.h
         include/OrbitGl/TextRendererInterface.h
         include/OrbitGl/ThreadBar.h
         include/OrbitGl/ThreadColor.h
         include/OrbitGl/ThreadStateBar.h
         include/OrbitGl/ThreadTrack.h
         include/OrbitGl/TimeGraph.h
         include/OrbitGl/TimeGraphLayout.h
         include/OrbitGl/TimelineInfoInterface.h
         include/OrbitGl/TimelineTicks.h
         include/OrbitGl/TimelineUi.h
         include/OrbitGl/Timer.h
         include/OrbitGl/TimerInfosIterator.h
         include/OrbitGl/TimerTrack.h
         include/OrbitGl/TracepointThreadBar.h
         include/OrbitGl/Track.h
         include/OrbitGl/TrackContainer.h
         include/OrbitGl/TrackControlInterface.h
         include/OrbitGl/TrackHeader.h
         include/OrbitGl/TrackManager.h
         include/OrbitGl/TrackRenderHelper.h
         include/OrbitGl/TrackTestData.h
         include/OrbitGl/TranslationStack.h
         include/OrbitGl/TriangleToggle.h
         include/OrbitGl/VariableTrack.h
         include/OrbitGl/VerticalSizer.h
         include/OrbitGl/Viewport.h)

target_sources(
  OrbitGl
  PRIVATE AccessibleCaptureViewElement.cpp
          AccessibleInterfaceProvider.cpp
          AccessibleTrack.cpp
          AccessibleTriangleToggle.cpp
          AnnotationTrack.cpp
          AsyncTrack.cpp
          BasicPageFaultsTrack.cpp
          Batcher.cpp
          BatchRenderGroup.cpp
          Button.cpp
          CallstackThreadBar.cpp
          CallTreeView.cpp
          CaptureStats.cpp
          CaptureViewElement.cpp
          CaptureWindow.cpp
          CGroupAndProcessMemoryTrack.cpp
          FormatCallstackForTooltip.cpp
          FrameTrack.cpp
          FrameTrackOnlineProcessor.cpp
          GlCanvas.cpp
          GlSlider.cpp
          GpuDebugMarkerTrack.cpp
          GpuSubmissionTrack.cpp
          GpuTrack.cpp
          GraphTrack.cpp
          GraphTrackDataAggregator.cpp
          IntrospectionWindow.cpp
          LineGraphTrack.cpp
          LiveFunctionsController.cpp
          ManualInstrumentationManager.cpp
          MajorPageFaultsTrack.cpp
          MemoryTrack.cpp
          MinorPageFaultsTrack.cpp
          MultivariateTimeSeries.cpp
          OpenGlBatcher.cpp
          OrbitApp.cpp
          QtTextRenderer.cpp
          PageFaultsTrack.cpp
          PickingManager.cpp
          PrimitiveAssembler.cpp
          SamplingReport.cpp
          SchedulerTrack.cpp
          SchedulingStats.cpp
          SelectionData.cpp
          SimpleTimings.cpp
          SymbolLoader.cpp
          SystemMemoryTrack.cpp
          TimeGraph.cpp
          TimelineTicks.cpp
          TimelineUi.cpp
          TimerInfosIterator.cpp
          TimerTrack.cpp
          ThreadBar.cpp
          ThreadColor.cpp
          ThreadStateBar.cpp
          ThreadTrack.cpp
          TracepointThreadBar.cpp
          Track.cpp
          TrackContainer.cpp
          TrackHeader.cpp
          TrackManager.cpp
          TrackRenderHelper.cpp
          TrackTestData.cpp
          TranslationStack.cpp
          TriangleToggle.cpp
          VerticalSizer.cpp
          Viewport.cpp)

target_include_directories(OrbitGl PUBLIC include/)

target_link_libraries(
  OrbitGl
  PUBLIC CaptureClient
         CaptureFileInfo
         ClientData
         ClientFlags
         ClientModel
         ClientServices
         ClientSymbols
         CodeReport
         Containers
         DataViews
         DisplayFormats
         GrpcProtos
         Http
         OrbitAccessibility
         OrbitBase
         OrbitPaths
         OrbitVersion
         PresetFile
         StringManager
         RemoteSymbolProvider
         SymbolProvider
         Symbols
         gte::gte
         Qt5::Gui)

add_custom_command(TARGET OrbitGl POST_BUILD COMMAND ${CMAKE_COMMAND}
                   -E copy ${CMAKE_CURRENT_SOURCE_DIR}/resources/stadia-default-frame-track.opr
                   ${CMAKE_BINARY_DIR}/bin/autopresets/stadia-default-frame-track.opr)

if(TARGET OpenGL::GLX AND TARGET OpenGL::OpenGL)
  target_link_libraries(OrbitGl PUBLIC OpenGL::GLX)
elseif(TARGET OpenGL::GL)
  target_link_libraries(OrbitGl PUBLIC OpenGL::GL)
endif()

add_executable(OrbitGlTests)

target_sources(OrbitGlTests PRIVATE
               include/OrbitGl/CaptureViewElementTester.h
               include/OrbitGl/MockBatcher.h
               include/OrbitGl/MockTextRenderer.h
               include/OrbitGl/PickingManagerTest.h)

target_sources(OrbitGlTests PRIVATE
               BatcherTest.cpp
               BatchRenderGroupTest.cpp
               ButtonTest.cpp
               CaptureStatsTest.cpp
               CaptureViewElementTest.cpp
               CaptureViewElementTester.cpp
               CaptureWindowTest.cpp
               CoreMathTest.cpp
               FormatCallstackForTooltipTest.cpp
               GlUtilsTest.cpp
               GpuTrackTest.cpp
               GraphTrackDataAggregatorTest.cpp
               MockBatcher.cpp
               MockTextRenderer.cpp
               MultivariateTimeSeriesTest.cpp
               OpenGlBatcherTest.cpp
               PageFaultsTrackTest.cpp
               PickingManagerTest.cpp
               PrimitiveAssemblerTest.cpp
               SimpleTimingsTest.cpp
               SliderTest.cpp
               ShortenStringWithEllipsisTest.cpp
               TimeGraphTest.cpp
               TimelineTicksTest.cpp
               TimelineUiTest.cpp
               TimerInfosIteratorTest.cpp
               TranslationStackTest.cpp
               ThreadTrackTest.cpp
               TrackHeaderTest.cpp
               TrackManagerTest.cpp
               ViewportTest.cpp)

target_link_libraries(
  OrbitGlTests
  PRIVATE OrbitGl
          GTest_Main)

register_test(OrbitGlTests)

