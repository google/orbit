# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitQt CXX)

add_executable(OrbitQt)

target_compile_options(OrbitQt PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  OrbitQt
  PRIVATE deploymentconfigurations.h
          eventloop.h
          orbitaboutdialog.h
          orbitcodeeditor.h
          orbitdataviewpanel.h
          orbitdisassemblydialog.h
          orbitglwidget.h
          orbitglwidgetwithheader.h
          orbitmainwindow.h
          orbitsamplingreport.h
          OrbitStartupWindow.h
          orbittablemodel.h
          orbittreeitem.h
          orbittreemodel.h
          orbittreeview.h
          orbitwatchwidget.h
          outputdialog.h
          processlauncherwidget.h
          resource.h
          servicedeploymanager.h
          showincludesdialog.h)

target_sources(
  OrbitQt
  PRIVATE deploymentconfigurations.cpp
          main.cpp
          orbitaboutdialog.cpp
          orbitaboutdialog.ui
          orbitcodeeditor.cpp
          orbitdataviewpanel.cpp
          orbitdisassemblydialog.cpp
          orbitglwidget.cpp
          orbitglwidgetwithheader.cpp
          orbitmainwindow.cpp
          orbitsamplingreport.cpp
          OrbitStartupWindow.cpp
          orbittablemodel.cpp
          orbittreeitem.cpp
          orbittreemodel.cpp
          orbittreeview.cpp
          orbitwatchwidget.cpp
          outputdialog.cpp
          processlauncherwidget.cpp
          servicedeploymanager.cpp
          showincludesdialog.cpp)

target_include_directories(OrbitQt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(OrbitQt PRIVATE OrbitCore OrbitGl OrbitGgp OrbitSshQt
                                      Qt5::Widgets Qt5::Core
                                      qtpropertybrowser::qtpropertybrowser)

include(${CMAKE_SOURCE_DIR}/cmake/version.cmake)
GenerateVersionFile("${CMAKE_CURRENT_BINARY_DIR}/version.h"
                    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in" OrbitQt)
target_sources(OrbitQt PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/version.h")
target_include_directories(OrbitQt PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

if(WIN32)
  set_target_properties(OrbitQt PROPERTIES WIN32_EXECUTABLE ON)

  GenerateVersionFile("${CMAKE_CURRENT_BINARY_DIR}/OrbitQt.rc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/OrbitQt.rc.in" OrbitQt)
  target_sources(OrbitQt PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/OrbitQt.rc)

  get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
  find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

  add_custom_command(
    TARGET OrbitQt
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E env PATH="${_qt_bin_dir}"
            "${WINDEPLOYQT_EXECUTABLE}" --pdb "$<TARGET_FILE:OrbitQt>"
    COMMENT "Running windeployqt...")
endif()

add_custom_command(TARGET OrbitQt POST_BUILD COMMAND ${CMAKE_COMMAND}
                   -E copy_directory ${CMAKE_SOURCE_DIR}/icons/
                   $<TARGET_FILE_DIR:OrbitQt>/icons)

set_target_properties(OrbitQt PROPERTIES OUTPUT_NAME "Orbit")
set_target_properties(OrbitQt PROPERTIES AUTOMOC ON)
set_target_properties(OrbitQt PROPERTIES AUTOUIC ON)
strip_symbols(OrbitQt)
