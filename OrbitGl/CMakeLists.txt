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
         FunctionsDataView.h
         Geometry.h
         GlCanvas.h
         GlobalsDataView.h
         GlPanel.h
         GlSlider.h
         GlUtils.h
         GraphTrack.h
         HomeWindow.h
         Images.h
         ImGuiOrbit.h
         ImmediateWindow.h
         LiveFunctionsDataView.h
         LogDataView.h
         mat4.h
         ModulesDataView.h
         OpenGl.h
         PickingManager.h
         PluginCanvas.h
         PluginManager.h
         ProcessesDataView.h
         RuleEditor.h
         SamplingReport.h
         SamplingReportDataView.h
         SchedulerTrack.h
         SessionsDataView.h
         shader.h
         TextBox.h
         TextRenderer.h
         ThreadTrack.h
         TimeGraph.h
         TimeGraphLayout.h
         Track.h
         TypesDataView.h)

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
          FunctionsDataView.cpp
          GlCanvas.cpp
          GlobalsDataView.cpp
          GlPanel.cpp
          GlSlider.cpp
          GlUtils.cpp
          GraphTrack.cpp
          HomeWindow.cpp
          ImGuiOrbit.cpp
          ImmediateWindow.cpp
          LiveFunctionsDataView.cpp
          LogDataView.cpp
          ModulesDataView.cpp
          PickingManager.cpp
          PluginCanvas.cpp
          PluginManager.cpp
          ProcessesDataView.cpp
          RuleEditor.cpp
          SamplingReport.cpp
          SamplingReportDataView.cpp
          SchedulerTrack.cpp
          SessionsDataView.cpp
          TextBox.cpp
          TextRenderer.cpp
          TimeGraph.cpp
          TimeGraphLayout.cpp
          Track.cpp
          ThreadTrack.cpp
          TypesDataView.cpp)

target_include_directories(OrbitGl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  OrbitGl
  PUBLIC OrbitCore
         OrbitAsm
         freetype-gl::freetype-gl
         OpenGL::GLU
         freetype::freetype
         imgui::imgui
         freeglut::freeglut
         capstone::capstone
         gte::gte
         glew::glew)

if(TARGET OpenGL::GLX AND TARGET OpenGL::OpenGL)
  target_link_libraries(OrbitGl PUBLIC OpenGL::GLX)
endif()

if(NOT WIN32)
  target_link_libraries(OrbitGl PRIVATE X11::X11 X11::Xi X11::Xxf86vm)
endif()
