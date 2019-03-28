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

# OrbitQt
set(MSVC_INCREMENTAL_DEFAULT ON)
project(OrbitQt)

# Disable incremental linking as it hides the real address of functions.  
# This is annoying mainly for the OrbitProlog and OrbitEpilog related code.
if(WIN32)
    set( MSVC_INCREMENTAL_YES_FLAG "/INCREMENTAL:NO")
    set( ORBIT_DEBUG_LINKER_FLAGS "/debug /INCREMENTAL:NO" )
    
    # debug
    message("Changing Debug linker flags from '${CMAKE_EXE_LINKER_FLAGS_DEBUG}' to '${ORBIT_DEBUG_LINKER_FLAGS}'")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG ${ORBIT_DEBUG_LINKER_FLAGS} )
    message("CMAKE_EXE_LINKER_FLAGS_DEBUG=${CMAKE_EXE_LINKER_FLAGS_DEBUG}")

    # rewithdebinfo
    PrintVars(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO)
    message("Changing RelWithDebInfo linker flags from '${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}' to '${ORBIT_DEBUG_LINKER_FLAGS}'")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO ${ORBIT_DEBUG_LINKER_FLAGS} )
endif()


# Stop compilation on first error
if(LINUX)
add_definitions(-Wfatal-errors)
endif()

if(WIN32)
if(NOT DEFINED ENV{QTDIR})
	MESSAGE(STATUS "QTDIR environment variable not set.
    Please QTDIR to your qt installation directory (ex:C:\Qt\5.12.1\msvc2017_64).")
endif()

set(QTDIR $ENV{QTDIR} CACHE STRING "Qt install path")
STRING(REGEX REPLACE "\\\\" "/" QTDIR ${QTDIR})
PrintVars(QTDIR)
list(APPEND CMAKE_PREFIX_PATH ${QTDIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${QTDIR}")
endif()

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

add_executable(OrbitQt WIN32
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
if(LINUX)
find_library(PNG_LIB libpng.a)
set(PNG_LIBRARIES ${PNG_LIB})
endif()
PrintVars(PNG)

#freetype-gl
#find_path(FREETYPE_GL_INCLUDE_DIR freetype-gl.h PATH_SUFFIXES freetype-gl)
if(WIN32)
find_library(FREETYPE_GL_LIB freetype-gl.lib)
endif()
if(LINUX)
#find_package(FREETYPE_GL)
find_library(FREETYPE_GL_LIB libfreetype-gl.a)
PrintVars(FREETYPE_GL)
endif()

#GLUT
find_package(GLUT REQUIRED)
PrintVars(GLUT)
if(LINUX)
find_package(CURL REQUIRED)
endif()

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

if(WIN32)

set(BREAKPAD_LIB optimized;${EXTERN_ROOT}/vcpkg/installed/x64-windows/lib/libbreakpad.lib;debug;${EXTERN_ROOT}/vcpkg/installed/x64-windows/debug/lib/libbreakpadd.lib)

set(ORBIT_DEPENDENCIES
    OrbitGl
    OrbitCore
    OrbitAsm
    OrbitDll
    Qt5::Widgets
    Qt5::Core
    dbghelp.lib
    ${BREAKPAD_LIB}
    ${EXTERN_ROOT}/vcpkg/installed/x64-windows/lib/libcurl.lib;
    ${EXTERN_ROOT}/vcpkg/installed/x64-windows/lib/imgui.lib;
    ${EXTERN_ROOT}/vcpkg/installed/x64-windows/lib/zlib.lib;
    ${EXTERN_ROOT}/vcpkg/installed/x64-windows/lib/freetype-gl.lib;
    ${EXTERN_ROOT}/vcpkg/installed/x64-windows/lib/freeglut.lib;
    ${EXTERN_ROOT}/vcpkg/installed/x64-windows/lib/glew32.lib;
    ${EXTERN_ROOT}/vcpkg/installed/x64-windows/lib/libpng16.lib;
    ${EXTERN_ROOT}/vcpkg/installed/x64-windows/lib/zlib.lib;
    ${EXTERN_ROOT}/vcpkg/installed/x64-windows/lib/freetype.lib;
    ${EXTERN_ROOT}/vcpkg/installed/x64-windows-static/lib/capstone.lib
)

endif()

if(LINUX)
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
    ${PNG_LIBRARIES}
#    ${FREETYPE_LIBRARY}
    ${FREETYPE_SYSTEM_LIB}
    Qt5::Widgets
    Qt5::Core
)
endif()

PrintVars(ORBIT_DEPENDENCIES)
target_link_libraries (OrbitQt
    ${ORBIT_DEPENDENCIES}
)

if(WIN32)
# Administrator privileges
SET_TARGET_PROPERTIES(OrbitQt PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' /SUBSYSTEM:WINDOWS")
add_custom_command(TARGET OrbitQt POST_BUILD
    
    # Qt
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:OrbitQt>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Gui> $<TARGET_FILE_DIR:OrbitQt>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:OrbitQt>

    # External
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXTERN_ROOT}/vcpkg/installed/x64-windows/bin/libcurl.dll  $<TARGET_FILE_DIR:OrbitQt> 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXTERN_ROOT}/vcpkg/installed/x64-windows/bin/imgui.dll    $<TARGET_FILE_DIR:OrbitQt> 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXTERN_ROOT}/vcpkg/installed/x64-windows/bin/zlib1.dll    $<TARGET_FILE_DIR:OrbitQt> 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXTERN_ROOT}/vcpkg/installed/x64-windows/bin/bz2.dll      $<TARGET_FILE_DIR:OrbitQt> 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXTERN_ROOT}/vcpkg/installed/x64-windows/bin/freeglut.dll $<TARGET_FILE_DIR:OrbitQt>     	
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXTERN_ROOT}/vcpkg/installed/x64-windows/bin/glew32.dll   $<TARGET_FILE_DIR:OrbitQt> 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXTERN_ROOT}/vcpkg/installed/x64-windows/bin/libpng16.dll $<TARGET_FILE_DIR:OrbitQt>     
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXTERN_ROOT}/vcpkg/installed/x64-windows/bin/freetype.dll $<TARGET_FILE_DIR:OrbitQt> 

    # OrbitDll
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/OrbitDll/$<CONFIG>/Orbit64.dll"        $<TARGET_FILE_DIR:OrbitQt>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ORBIT_ROOT}/build/x86/OrbitDll/Release/Orbit32.dll"      $<TARGET_FILE_DIR:OrbitQt>

    # Icon
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ORBIT_ROOT}/OrbitQt/orbit.ico"                           $<TARGET_FILE_DIR:OrbitQt>
)

set_target_properties(OrbitQt PROPERTIES OUTPUT_NAME "Orbit")

PrintVars(qt)

endif()
