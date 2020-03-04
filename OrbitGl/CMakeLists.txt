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

if(WITH_GUI)
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
           glew::glew)
else()
  # Don't build this target by default. It will not compile in a WITH_GUI=OFF
  # setup.
  set_target_properties(OrbitGl PROPERTIES EXCLUDE_FROM_ALL ON)
endif()

if(TARGET OpenGL::GLX AND WITH_GUI)
  target_link_libraries(OrbitGl PUBLIC OpenGL::GLX)
endif()

if(NOT WIN32 AND WITH_GUI)
  target_link_libraries(OrbitGl PRIVATE X11::X11 X11::Xi X11::Xxf86vm)
endif()


# OrbitNoGl:
# This target is a temporary solution to build OrbitService
# without a dependency to OpenGl.
# According to my tests, this does not affect the behaviour
# of the collector, but I might be wrong.
# I tested the following things:
# - Starting the collector
# - Connecting with the GUI
# - Listing processes
# - Sampling a process
# - Load debug symbols
# - Hook a function
# - Verify that hooked function appears.
add_library(OrbitNoGl STATIC)


get_target_property(PRIV_SOURCES OrbitGl SOURCES)
list(
  REMOVE_ITEM
  PRIV_SOURCES
  shader.cpp
  EventTrack.cpp
  BlackBoard.cpp
  CaptureSerializer.cpp
  CaptureWindow.cpp
  Card.cpp
  PickingManager.cpp
  GlUtils.cpp
  ImmediateWindow.cpp
  GlPanel.cpp
  GlCanvas.cpp
  HomeWindow.cpp
  ImGuiOrbit.cpp
  GlSlider.cpp
  TextRenderer.cpp
  TextBox.cpp
  Track.cpp
  RuleEditor.cpp
  PluginCanvas.cpp
  TimeGraph.cpp
  ThreadTrack.cpp)
target_sources(OrbitNoGl PRIVATE "${PRIV_SOURCES}")

get_target_property(PUB_SOURCES OrbitGl INTERFACE_SOURCES)
target_sources(OrbitNoGl PUBLIC "${PUB_SOURCES}")

get_target_property(PUB_INC OrbitGl INCLUDE_DIRECTORIES)
set_target_properties(OrbitNoGl PROPERTIES INCLUDE_DIRECTORIES "${PUB_INC}")

get_target_property(INT_INC OrbitGl INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(OrbitNoGl PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                           "${INT_INC}")

target_link_libraries(OrbitNoGl PUBLIC OrbitCore OrbitAsm capstone::capstone)

target_compile_definitions(OrbitNoGl PUBLIC NOGL)
