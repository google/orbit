# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitGl CXX)
add_library(OrbitGl STATIC)

target_compile_options(OrbitGl PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  OrbitGl
  PUBLIC App.h
         AsyncTrack.h
         Batcher.h
         CallStackDataView.h
         CaptureWindow.h
         CodeReport.h
         CoreMath.h
         DataView.h
         DataViewTypes.h
         Disassembler.h
         DisassemblyReport.h
         EventTrack.h
         FramePointerValidatorClient.h
         FunctionsDataView.h
         Geometry.h
         GlCanvas.h
         GlPanel.h
         GlSlider.h
         GlUtils.h
         GpuTrack.h
         GraphTrack.h
         Images.h
         ImGuiOrbit.h
         LiveFunctionsController.h
         LiveFunctionsDataView.h
         ManualInstrumentationManager.h
         ModulesDataView.h
         OpenGl.h
         PickingManager.h
         PresetLoadState.h
         PresetsDataView.h
         ProcessesDataView.h
         SamplingReport.h
         SamplingReportDataView.h
         SchedulerTrack.h
         StatusListener.h
         TextBox.h
         TextRenderer.h
         ThreadTrack.h
         TimeGraph.h
         TimeGraphLayout.h
         TimerChain.h
         TimerInfosIterator.h
         TopDownView.h
         TimerTrack.h
         Track.h
         TriangleToggle.h
         TracepointsDataView.h
         TracepointTrack.h)

target_sources(
  OrbitGl
  PRIVATE App.cpp
          AsyncTrack.cpp
          Batcher.cpp
          CallStackDataView.cpp
          CaptureWindow.cpp
          DataManager.cpp
          DataView.cpp
          Disassembler.cpp
          DisassemblyReport.cc
          EventTrack.cpp
          FramePointerValidatorClient.cpp
          LiveFunctionsController.cpp
          FunctionsDataView.cpp
          GlCanvas.cpp
          GlPanel.cpp
          GlSlider.cpp
          GlUtils.cpp
          GpuTrack.cpp
          GraphTrack.cpp
          ImGuiOrbit.cpp
          LiveFunctionsDataView.cpp
          ManualInstrumentationManager.cpp
          ModulesDataView.cpp
          PickingManager.cpp
          PresetsDataView.cpp
          ProcessesDataView.cpp
          SamplingReport.cpp
          SamplingReportDataView.cpp
          SchedulerTrack.cpp
          TextRenderer.cpp
          TimeGraph.cpp
          TimeGraphLayout.cpp
          TimerChain.cpp
          TimerInfosIterator.cpp
          TimerTrack.cpp
          ThreadTrack.cpp
          TopDownView.cpp
          Track.cpp
          TriangleToggle.cpp
          TracepointsDataView.cpp
          TracepointTrack.cpp)

target_include_directories(OrbitGl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  OrbitGl
  PUBLIC OrbitCore
         OrbitCaptureClient
         OrbitClientModel
         OrbitClientServices
         OrbitProtos
         freetype-gl::freetype-gl
         OpenGL::GLU
         freetype::freetype
         imgui::imgui
         capstone::capstone
         gte::gte
         glew::glew
         Outcome::Outcome
         stb::stb)

if(TARGET OpenGL::GLX AND TARGET OpenGL::OpenGL)
  target_link_libraries(OrbitGl PUBLIC OpenGL::GLX)
endif()

add_executable(OrbitGlTests)

target_compile_options(OrbitGlTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitGlTests PRIVATE
               PickingManagerTest.h)

target_sources(OrbitGlTests PRIVATE
               BatcherTest.cpp
               PickingManagerTest.cpp
               ScopedStatusTest.cpp
               TimerInfosIteratorTest.cpp)

target_link_libraries(
  OrbitGlTests
  PRIVATE OrbitGl
          GTest::Main)

register_test(OrbitGlTests)

add_fuzzer(CaptureDeserializerLoadFuzzer CaptureDeserializerLoadFuzzer.cpp)
target_link_libraries(CaptureDeserializerLoadFuzzer
                      PRIVATE OrbitGl libprotobuf-mutator::libprotobuf-mutator)

add_fuzzer(DataManagerUpdateModuleInfosFuzzer
           DataManagerUpdateModuleInfosFuzzer.cpp)
target_link_libraries(DataManagerUpdateModuleInfosFuzzer
                      PRIVATE OrbitGl libprotobuf-mutator::libprotobuf-mutator)

