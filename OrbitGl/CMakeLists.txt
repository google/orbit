cmake_minimum_required(VERSION 3.6)

# Orbit directories
set(ORBIT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(EXTERN_ROOT "${ORBIT_ROOT}/external")
set(ORBIT_UTILS "${ORBIT_ROOT}/OrbitUtils")

# Orbit utils (PrintVars, PrintAllVars...)
include(${ORBIT_UTILS}/utils.cmake)

# Vcpkg toolchain
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${EXTERN_ROOT}/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Linux
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# OrbitGl
project(OrbitGl)

# Stop compilation on first error
add_definitions(-Wfatal-errors)

#freetype
find_package(Freetype REQUIRED)
PrintVars(FREETYPE)

find_package(PNG REQUIRED)
PrintVars(PNG)

#find_package(freeglut REQUIRED)
find_path(FREEGLUT_INCLUDE_DIR GL/freeglut.h)

if(WIN32)
    find_library(FREEGLUT_LIB freeglut_static.lib)
    #HINTS("external/vcpkg/buildtrees/freeglut/x64-windows-static-rel/lib/Release")
endif()

if(LINUX)
find_library(FREEGLUT_LIB libglut.a)
PrintVars(FREEGLUT)
endif()

# Glew
find_package(glew REQUIRED)
PrintVars(GLEW)

# Freetype-gl
find_path(FREETYPE_GL_INCLUDE_DIR freetype-gl.h PATH_SUFFIXES freetype-gl)
PrintVars(FREETYPE_GL_INCLUDE_DIR)
# if(WIN32)
# find_library(FREETYPE_GL_LIB freetype-gl.lib)
# endif()
# if(LINUX)
# find_library(FREETYPE_GL_LIB libfreetype-gl.a)
# endif()

# imgui
find_path(IMGUI_INCLUDE_DIR imgui.h) 
if(WIN32)
find_library(IMGUI_LIB imgui.lib)
endif()
if(LINUX)
find_library(IMGUI_LIB libimgui.a)
PrintVars(IMGUI_LIB)
endif()

set(HEADERS
    App.h
    Batcher.h
    BlackBoard.h
    CallStackDataView.h
    CaptureSerializer.h
    CaptureWindow.h
    Card.h
    CoreMath.h
    DataView.h
    DataViewTypes.h
#    Debugger.h
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
    ThreadDataViewGl.h
    ThreadView.h
    TimeGraph.h
    TimeGraphLayout.h
    TypeDataView.h
)

set(SOURCES
    shader.cpp
    mat4.cpp

    App.cpp
    Batcher.cpp
    BlackBoard.cpp
    CallStackDataView.cpp
    CaptureSerializer.cpp
    CaptureWindow.cpp
    Card.cpp
    DataView.cpp
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
    ThreadDataViewGl.cpp
    ThreadView.cpp
    TimeGraph.cpp
    TimeGraphLayout.cpp
    TypeDataView.cpp
    #Debugger.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

include_directories("${_VCPKG_INSTALLED_DIR}/x64-linux/include")

add_library(OrbitGl STATIC ${SOURCES} ${HEADERS})

find_package(X11 REQUIRED)
#PrintVars(X11)

target_include_directories(OrbitGl
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
PRIVATE
    ${FREETYPE_INCLUDE_DIRS}
    ${FREETYPE_GL_INCLUDE_DIR}
    ${CAPSTONE_INCLUDE_DIR}
    ${FREEGLUT_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CEREAL_INCLUDE_DIR}
    ${EXTERN_ROOT}/concurrentqueue
    ${EXTERN_ROOT}/multicore/common/
    ${EXTERN_ROOT}/xxHash-r42
#    ${EXTERN_ROOT}/oqpi/include
    ${EXTERN_ROOT}/minhook/include/
    ../OrbitCore/
    ..
)

find_package(OpenGL REQUIRED)
PrintVars(OPENGL)

include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

PrintVars(PNG_LIBRARIES)

set(ORBITGL_DEPENDENCIES
OrbitCore
#OrbitAsm
${PNG_LIBRARIES}
${FREEGLUT_LIB}
${IMGUI_LIB}
${GLEW_LIBRARIES}
${FREETYPE_LIB}
${FREETYPE_GL_LIB}
${GLUT_LIBRARIES}
${CAPSTONE_LIBRARIES}
${OPENGL_LIBRARIES}
${X11_LIBRARIES}
)

PrintVars(ORBITGL_DEPENDENCIES)

target_link_libraries(OrbitGl ${ORBITGL_DEPENDENCIES})
