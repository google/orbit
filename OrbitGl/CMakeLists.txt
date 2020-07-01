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
         CoreMath.h
         DataView.h
         DataViewTypes.h
         Debugger.h
         Disassembler.h
         EventTrack.h
         FramePointerValidatorClient.h
         FunctionsDataView.h
         Geometry.h
         GlCanvas.h
         GlobalsDataView.h
         GlPanel.h
         GlSlider.h
         GlUtils.h
         GpuTrack.h
         GraphTrack.h
         HomeWindow.h
         Images.h
         ImGuiOrbit.h
         LiveFunctionsDataView.h
         LogDataView.h
         ModulesDataView.h
         OpenGl.h
         PickingManager.h
         PluginCanvas.h
         PluginManager.h
         ProcessesDataView.h
         SamplingReport.h
         SamplingReportDataView.h
         SchedulerTrack.h
         SessionsDataView.h
         TextBox.h
         TextRenderer.h
         ThreadTrack.h
         TimeGraph.h
         TimeGraphLayout.h
         TimerChain.h
         Track.h
         TriangleToggle.h
         TypesDataView.h)

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
          Debugger.cpp
          Disassembler.cpp
          EventTrack.cpp
          FramePointerValidatorClient.cpp
          FunctionsDataView.cpp
          GlCanvas.cpp
          GlobalsDataView.cpp
          GlPanel.cpp
          GlSlider.cpp
          GlUtils.cpp
          GpuTrack.cpp
          GraphTrack.cpp
          HomeWindow.cpp
          ImGuiOrbit.cpp
          LiveFunctionsDataView.cpp
          LogDataView.cpp
          ModulesDataView.cpp
          PickingManager.cpp
          PluginCanvas.cpp
          PluginManager.cpp
          ProcessManager.cpp
          ProcessesDataView.cpp
          SamplingReport.cpp
          SamplingReportDataView.cpp
          SchedulerTrack.cpp
          SessionsDataView.cpp
          TextBox.cpp
          TextRenderer.cpp
          TimeGraph.cpp
          TimeGraphLayout.cpp
          TimerChain.cpp
          ThreadTrack.cpp
          Track.cpp
          TriangleToggle.cpp
          TypesDataView.cpp)

target_include_directories(OrbitGl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  OrbitGl
  PUBLIC OrbitCore
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
