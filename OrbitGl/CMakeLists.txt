cmake_minimum_required(VERSION 3.15)

project(OrbitGl CXX)
add_library(OrbitGl STATIC)

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
         FunctionDataView.h
         Geometry.h
         GlCanvas.h
         GlobalDataView.h
         GlPanel.h
         GlSlider.h
         GlUtils.h
         HomeWindow.h
         Images.h
         ImGuiOrbit.h
         ImmediateWindow.h
         LiveFunctionDataView.h
         LogDataView.h
         mat4.h
         ModuleDataView.h
         OpenGl.h
         PickingManager.h
         PluginCanvas.h
         PluginManager.h
         ProcessDataView.h
         resource.h
         RuleEditor.h
         SamplingReport.h
         SamplingReportDataView.h
         SessionsDataView.h
         shader.h
         TextBox.h
         TextRenderer.h
         ThreadTrack.h
         ThreadTrackMap.h
         TimeGraph.h
         TimeGraphLayout.h
         Track.h
         TypeDataView.h)

target_sources(
  OrbitGl
  PRIVATE shader.cpp
          mat4.cpp
          App.cpp
          Batcher.cpp
          BlackBoard.cpp
          CallStackDataView.cpp
          CaptureSerializer.cpp
          CaptureWindow.cpp
          Card.cpp
          DataView.cpp
          Debugger.cpp
          Disassembler.cpp
          EventTrack.cpp
          FunctionDataView.cpp
          GlCanvas.cpp
          GlobalDataView.cpp
          GlPanel.cpp
          GlSlider.cpp
          GlUtils.cpp
          HomeWindow.cpp
          ImGuiOrbit.cpp
          ImmediateWindow.cpp
          LiveFunctionDataView.cpp
          LogDataView.cpp
          ModuleDataView.cpp
          PickingManager.cpp
          PluginCanvas.cpp
          PluginManager.cpp
          ProcessDataView.cpp
          RuleEditor.cpp
          SamplingReport.cpp
          SamplingReportDataView.cpp
          SessionsDataView.cpp
          TextBox.cpp
          TextRenderer.cpp
          TimeGraph.cpp
          TimeGraphLayout.cpp
          Track.cpp
          ThreadTrack.cpp
          TypeDataView.cpp)

target_include_directories(OrbitGl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  OrbitGl
  PUBLIC OrbitCore
         OrbitAsm
         freetype-gl::freetype-gl
         OpenGL::GL
         Freetype::Freetype
         GLEW::glew_s
         imgui::imgui
         GLUT::GLUT
         capstone::capstone)

if(NOT WIN32)
  target_link_libraries(OrbitGl PRIVATE X11::X11 X11::Xi X11::Xxf86vm)
endif()
