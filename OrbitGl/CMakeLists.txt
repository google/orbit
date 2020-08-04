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
         Batcher.h
         BlackBoard.h
         CallStackDataView.h
         CaptureSerializer.h
         CaptureWindow.h
         Card.h
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
         HomeWindow.h
         Images.h
         ImGuiOrbit.h
         LiveFunctionsController.h
         LiveFunctionsDataView.h
         ModulesDataView.h
         OpenGl.h
         PickingManager.h
         PresetsDataView.h
         ProcessesDataView.h
         SamplingReport.h
         SamplingReportDataView.h
         SchedulerTrack.h
         TextBox.h
         TextRenderer.h
         ThreadTrack.h
         TimeGraph.h
         TimeGraphLayout.h
         TimerChain.h
         TopDownView.h
         TimerTrack.h
         Track.h
         TriangleToggle.h)

target_sources(
  OrbitGl
  PRIVATE App.cpp
          Batcher.cpp
          BlackBoard.cpp
          CallStackDataView.cpp
          CaptureSerializer.cpp
          CaptureWindow.cpp
          Card.cpp
          CrashManager.cpp
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
          HomeWindow.cpp
          ImGuiOrbit.cpp
          LiveFunctionsDataView.cpp
          ModulesDataView.cpp
          PickingManager.cpp
          PresetsDataView.cpp
          ProcessManager.cpp
          ProcessesDataView.cpp
          SamplingReport.cpp
          SamplingReportDataView.cpp
          SchedulerTrack.cpp
          TextBox.cpp
          TextRenderer.cpp
          TimeGraph.cpp
          TimeGraphLayout.cpp
          TimerChain.cpp
          TimerTrack.cpp
          ThreadTrack.cpp
          TopDownView.cpp
          Track.cpp
          TriangleToggle.cpp)

target_include_directories(OrbitGl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  OrbitGl
  PUBLIC OrbitCore
         OrbitCaptureClient
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

if(NOT WIN32)
  target_link_libraries(OrbitGl PRIVATE X11::X11 X11::Xi X11::Xxf86vm)
endif()

add_executable(OrbitGlTests)

target_sources(OrbitGlTests PRIVATE
    PickingManagerTest.cpp)

target_link_libraries(
  OrbitGlTests
  PRIVATE OrbitGl
          GTest::Main)

register_test(OrbitGlTests)

add_fuzzer(CaptureSerializerLoadFuzzer CaptureSerializerLoadFuzzer.cpp)
target_link_libraries(CaptureSerializerLoadFuzzer PRIVATE OrbitGl)

add_fuzzer(DataManagerUpdateModuleInfosFuzzer
           DataManagerUpdateModuleInfosFuzzer.cpp)
target_link_libraries(DataManagerUpdateModuleInfosFuzzer
                      PRIVATE OrbitGl libprotobuf-mutator::libprotobuf-mutator)

add_fuzzer(CaptureEventProcessorProcessEventsFuzzer CaptureEventProcessorProcessEventsFuzzer.cpp)
target_link_libraries(
  CaptureEventProcessorProcessEventsFuzzer
  PRIVATE OrbitGl libprotobuf-mutator::libprotobuf-mutator)
