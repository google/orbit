cmake_minimum_required(VERSION 3.15)

project(OrbitGl CXX)
add_library(OrbitGl STATIC)

set(HEADERS include/OrbitGl)

target_sources(
  OrbitGl
  PUBLIC ${HEADERS}/App.h
         ${HEADERS}/Batcher.h
         ${HEADERS}/BlackBoard.h
         ${HEADERS}/CallStackDataView.h
         ${HEADERS}/CaptureSerializer.h
         ${HEADERS}/CaptureWindow.h
         ${HEADERS}/Card.h
         ${HEADERS}/CoreMath.h
         ${HEADERS}/DataView.h
         ${HEADERS}/DataViewTypes.h
         ${HEADERS}/Debugger.h
         ${HEADERS}/Disassembler.h
         ${HEADERS}/EventTrack.h
         ${HEADERS}/FunctionDataView.h
         ${HEADERS}/Geometry.h
         ${HEADERS}/GlCanvas.h
         ${HEADERS}/GlobalDataView.h
         ${HEADERS}/GlPanel.h
         ${HEADERS}/GlSlider.h
         ${HEADERS}/GlUtils.h
         ${HEADERS}/HomeWindow.h
         ${HEADERS}/Images.h
         ${HEADERS}/ImGuiOrbit.h
         ${HEADERS}/ImmediateWindow.h
         ${HEADERS}/LiveFunctionDataView.h
         ${HEADERS}/LogDataView.h
         ${HEADERS}/mat4.h
         ${HEADERS}/ModuleDataView.h
         ${HEADERS}/OpenGl.h
         ${HEADERS}/PickingManager.h
         ${HEADERS}/PluginCanvas.h
         ${HEADERS}/PluginManager.h
         ${HEADERS}/ProcessDataView.h
         ${HEADERS}/RuleEditor.h
         ${HEADERS}/SamplingReport.h
         ${HEADERS}/SamplingReportDataView.h
         ${HEADERS}/SessionsDataView.h
         ${HEADERS}/shader.h
         ${HEADERS}/TextBox.h
         ${HEADERS}/TextRenderer.h
         ${HEADERS}/ThreadTrack.h
         ${HEADERS}/ThreadTrackMap.h
         ${HEADERS}/TimeGraph.h
         ${HEADERS}/TimeGraphLayout.h
         ${HEADERS}/Track.h
         ${HEADERS}/TypeDataView.h)

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

target_include_directories(OrbitGl PUBLIC include/)
target_include_directories(OrbitGl PRIVATE include/OrbitGl/)

target_link_libraries(
  OrbitGl
  PUBLIC OrbitCore
         OrbitAsm
         freetype-gl::freetype-gl
         OpenGL::GL
         freetype
         imgui::imgui
         GLUT::GLUT
         capstone::capstone
         gte::gte)

if(NOT WIN32)
  target_link_libraries(OrbitGl PRIVATE X11::X11 X11::Xi X11::Xxf86vm
                                        GLEW::glew_s)
else()
  target_link_libraries(OrbitGl PRIVATE GLEW::glew)
endif()
