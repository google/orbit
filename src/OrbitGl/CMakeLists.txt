# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitGl CXX)
add_library(OrbitGl STATIC)

target_compile_options(OrbitGl PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  OrbitGl
  PUBLIC AccessibleTimeGraph.h
         App.h
         AsyncTrack.h
         Batcher.h
         BlockChain.h
         CallStackDataView.h
         CallTreeView.h
         CaptureWindow.h
         CaptureViewElement.h
         CodeReport.h
         CoreMath.h
         DataView.h
         DataViewTypes.h
         Disassembler.h
         DisassemblyReport.h
         EventTrack.h
         FramePointerValidatorClient.h
         FrameTrack.h
         FrameTrackOnlineProcessor.h
         FunctionsDataView.h
         Geometry.h
         GlCanvas.h
         GlSlider.h
         GlUtils.h
         GpuTrack.h
         GraphTrack.h
         Images.h
         ImGuiOrbit.h
         IntrospectionWindow.h
         LiveFunctionsController.h
         LiveFunctionsDataView.h
         ManualInstrumentationManager.h
         ModulesDataView.h
         OpenGl.h
         PickingManager.h
         PresetLoadState.h
         PresetsDataView.h
         SamplingReport.h
         SamplingReportDataView.h
         SchedulerTrack.h
         ScopeTree.h
         SourceCodeReport.h
         StatusListener.h
         TextBox.h
         TextRenderer.h
         ThreadBar.h
         ThreadStateTrack.h
         ThreadTrack.h
         TimeGraph.h
         TimeGraphLayout.h
         Timer.h
         TimerChain.h
         TimerInfosIterator.h
         TimerTrack.h
         Track.h
         TrackAccessibility.h
         TrackManager.h
         TriangleToggle.h
         TracepointsDataView.h
         TracepointTrack.h)

target_sources(
  OrbitGl
  PRIVATE AccessibleTimeGraph.cpp
          App.cpp
          AsyncTrack.cpp
          Batcher.cpp
          CallStackDataView.cpp
          CallTreeView.cpp
          CaptureWindow.cpp
          CaptureViewElement.cpp
          DataManager.cpp
          DataView.cpp
          Disassembler.cpp
          DisassemblyReport.cpp
          EventTrack.cpp
          FramePointerValidatorClient.cpp
          FrameTrack.cpp
          FrameTrackOnlineProcessor.cpp
          LiveFunctionsController.cpp
          FunctionsDataView.cpp
          GlCanvas.cpp
          GlSlider.cpp
          GlUtils.cpp
          GpuTrack.cpp
          GraphTrack.cpp
          ImGuiOrbit.cpp
          IntrospectionWindow.cpp
          LiveFunctionsDataView.cpp
          ManualInstrumentationManager.cpp
          ModulesDataView.cpp
          PickingManager.cpp
          PresetsDataView.cpp
          SamplingReport.cpp
          SamplingReportDataView.cpp
          SchedulerTrack.cpp
          SourceCodeReport.cpp
          TextRenderer.cpp
          TimeGraph.cpp
          TimeGraphLayout.cpp
          TimerChain.cpp
          TimerInfosIterator.cpp
          TimerTrack.cpp
          ThreadStateTrack.cpp
          ThreadTrack.cpp
          Track.cpp
          TrackAccessibility.cpp
          TrackManager.cpp
          TriangleToggle.cpp
          TracepointsDataView.cpp
          TracepointTrack.cpp)

target_include_directories(OrbitGl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  OrbitGl
  PUBLIC GrpcProtos
         MetricsUploader
         OrbitAccessibility
         OrbitCore
         OrbitCaptureClient
         OrbitClientData
         OrbitClientModel
         OrbitClientServices
         CONAN_PKG::Outcome
         CONAN_PKG::capstone
         CONAN_PKG::freetype-gl
         CONAN_PKG::freetype
         CONAN_PKG::glew
         CONAN_PKG::imgui
         gte::gte
         stb::stb)

if(TARGET OpenGL::GLX AND TARGET OpenGL::OpenGL)
  target_link_libraries(OrbitGl PUBLIC OpenGL::GLX)
elseif(TARGET OpenGL::GL)
  target_link_libraries(OrbitGl PUBLIC OpenGL::GL)
endif()

add_executable(OrbitGlTests)

target_compile_options(OrbitGlTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitGlTests PRIVATE
               PickingManagerTest.h)

target_sources(OrbitGlTests PRIVATE
               BatcherTest.cpp
               BlockChainTest.cpp
               GlUtilsTest.cpp
               GpuTrackTest.cpp
               PickingManagerTest.cpp
               ScopedStatusTest.cpp
               ScopeTreeTest.cpp
               SliderTest.cpp
               TimerInfosIteratorTest.cpp
               ClientFlags.cpp)

target_link_libraries(
  OrbitGlTests
  PRIVATE OrbitGl
          GTest::Main)

register_test(OrbitGlTests)
