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

# Linux
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# OrbitGl
project(OrbitGl)

# Stop compilation on first error
add_definitions(-Wfatal-errors)

if(WIN32)
set(QTDIR "C:/Qt/5.11.2/msvc2017_64")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${QTDIR}")

find_package(Qt5 CONFIG REQUIRED 
	HINTS "${QTDIR}" 
	COMPONENTS Widgets Core
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(HEADERS
    licensedialog.h
    orbitcodeeditor.h
    orbitdataviewpanel.h
    orbitdiffdialog.h
    orbitdisassemblydialog.h
    orbitglwidget.h
    orbitglwidgetwithheader.h
    orbitmainwindow.h
    orbitsamplingreport.h
    orbittablemodel.h
    orbittreeitem.h
    orbittreemodel.h
    orbittreeview.h
    orbitvisualizer.h
    orbitwatchwidget.h
    outputdialog.h
    processlauncherwidget.h
    resource.h
    showincludesdialog.h
)
set(SOURCES
    licensedialog.cpp
    main.cpp
    orbitcodeeditor.cpp
    orbitdataviewpanel.cpp
    orbitdiffdialog.cpp
    orbitdisassemblydialog.cpp
    orbitglwidget.cpp
    orbitglwidgetwithheader.cpp
    orbitmainwindow.cpp
    orbitsamplingreport.cpp
    orbittablemodel.cpp
    orbittreeitem.cpp
    orbittreemodel.cpp
    orbittreeview.cpp
    orbitvisualizer.cpp
    orbitwatchwidget.cpp
    outputdialog.cpp
    processlauncherwidget.cpp
    showincludesdialog.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

set(qtpropertybrowser_HEADERS
    qtpropertybrowser/qtbuttonpropertybrowser.h
    qtpropertybrowser/qteditorfactory.h
    qtpropertybrowser/qtgroupboxpropertybrowser.h
    qtpropertybrowser/qtpropertybrowser.h
    qtpropertybrowser/qtpropertybrowserutils_p.h
    qtpropertybrowser/qtpropertymanager.h
    qtpropertybrowser/qttreepropertybrowser.h
    qtpropertybrowser/qtvariantproperty.h
)
set(qtpropertybrowser_SOURCES
    qtpropertybrowser/qtbuttonpropertybrowser.cpp
    qtpropertybrowser/qteditorfactory.cpp
    qtpropertybrowser/qtgroupboxpropertybrowser.cpp
    qtpropertybrowser/qtpropertybrowser.cpp
    qtpropertybrowser/qtpropertybrowserutils.cpp
    qtpropertybrowser/qtpropertymanager.cpp
    qtpropertybrowser/qttreepropertybrowser.cpp
    qtpropertybrowser/qtvariantproperty.cpp
)
source_group("qtpropertybrowser\\Header Files" FILES ${qtpropertybrowser_HEADERS})
source_group("qtpropertybrowser\\Source Files" FILES ${qtpropertybrowser_SOURCES})

add_executable(OrbitQt 
    ${SOURCES} ${HEADERS} 
	${qtpropertybrowser_SOURCES} ${qtpropertybrowser_HEADERS}
)

find_package(ZLIB REQUIRED)
PrintVars(ZLIB)

find_package(X11 REQUIRED)
#PrintVars(X11)

find_path(IMGUI_INCLUDE_DIR imgui.h) 
if(WIN32)
find_library(IMGUI_LIB imgui.lib)
endif()
if(LINUX)
find_library(IMGUI_LIB libimgui.a)
endif()

#find_package(FREETYPE REQUIRED)
#find_library(FREETYPE_LIB libfreetype.a)
find_library(FREETYPE_SYSTEM_LIB freetype)
PrintVars(FREETYPE_SYSTEM_LIB)

# Glew
find_package(GLEW REQUIRED)
PrintVars(GLEW)

#Libpng
find_package(PNG REQUIRED)
#find_library(PNG_LIB libpng.a)
set( PNG_LIB_ORBIT "/home/pierric/dev/orbitprofiler/external/vcpkg/installed/x64-linux/lib/libpng16.a")
PrintVars(PNG)

#freetype-gl
#find_path(FREETYPE_GL_INCLUDE_DIR freetype-gl.h PATH_SUFFIXES freetype-gl)
if(WIN32)
find_library(FREETYPE_GL_LIB freetype-gl.lib)
endif()
if(LINUX)
#find_package(FREETYPE_GL)
#find_library(FREETYPE_GL_LIB libfreetype-gl.a)
set(FREETYPE_GL_LIB "/home/pierric/dev/orbitprofiler/external/vcpkg/installed/x64-linux/lib/libfreetype-gl.a")
PrintVars(FREETYPE_GL)
endif()

#GLUT
find_package(GLUT REQUIRED)
PrintVars(GLUT)
find_package(CURL REQUIRED)

#imgui
find_path(IMGUI_INCLUDE_DIR imgui.h) 
if(WIN32)
find_library(IMGUI_LIB imgui.lib)
endif()
if(LINUX)
find_library(IMGUI_LIB libimgui.a)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	set_target_properties(OrbitQt PROPERTIES COMPILE_FLAGS /wd4127)
endif()

set(ORBIT_PNG_LIB "/home/pierric/dev/orbitprofiler/external/vcpkg/buildtrees/libpng/x64-linux-rel/libpng16.a")

target_include_directories(OrbitQt
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
PRIVATE
    #${PNG_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${FREEGLUT_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
    ${FREETYPE_GL_INCLUDE_DIR}
    ${CAPSTONE_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CEREAL_INCLUDE_DIR}
    ${X11_INCLUDE_DIR}
    ${EXTERN_ROOT}/concurrentqueue
    ${EXTERN_ROOT}/multicore/common/
    ${EXTERN_ROOT}/xxHash-r42
#    ${EXTERN_ROOT}/oqpi/include
    ${EXTERN_ROOT}/minhook/include/
    ../OrbitCore/
    ..
)

PrintVars(PNG_LIBRARIES)
set(ORBIT_DEPENDENCIES
    OrbitGl
	OrbitCore
	${CURL_LIBRARIES}
    ${BREAKPAD_LIB}
    ${IMGUI_LIB}
    ${ZLIB_LIBRARIES}
    ${FREETYPE_GL_LIB}
    ${GLUT_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${X11_LIBRARIES}
    #${PNG_LIBRARIES}
    ${ORBIT_PNG_LIB}
    ${FREETYPE_SYSTEM_LIB}
	Qt5::Widgets
	Qt5::Core
)

# ${CURL_LIBRARIES}
#     ${BREAKPAD_LIB}
#     ${IMGUI_LIB}
#     ${ZLIB_LIBRARIES}
#     #${ORBIT_PNG_LIB}
#     ${FREETYPE_GL_LIB}
#     ${TEST_freetype_LIB}
#     ${OPENGL_LIBRARIES}
#     ${PNG_LIBRARIES}
#     ${GLUT_LIBRARIES}
#     ${GLEW_LIBRARIES}
#     ${X11_LIBRARIES}

PrintVars(ORBIT_DEPENDENCIES)
target_link_libraries (OrbitQt
    ${ORBIT_DEPENDENCIES}
)
