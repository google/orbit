# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(Orbit CXX)

add_executable(Orbit main.cpp)
target_compile_options(Orbit PRIVATE ${STRICT_COMPILE_FLAGS})
target_link_libraries(Orbit PRIVATE OrbitQt)
target_include_directories(Orbit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(WITH_CRASH_HANDLING)
  target_link_libraries(Orbit PRIVATE CrashHandler)
  target_compile_definitions(Orbit PRIVATE ORBIT_CRASH_HANDLING)
endif()

if(WIN32)
  set_target_properties(Orbit PROPERTIES WIN32_EXECUTABLE ON)

  include(${CMAKE_SOURCE_DIR}/cmake/version.cmake)
  GenerateVersionFile("${CMAKE_CURRENT_BINARY_DIR}/Orbit.rc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/Orbit.rc.in" Orbit)
  target_sources(Orbit PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/Orbit.rc)

  get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
  find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

  add_custom_command(
    TARGET Orbit
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E env PATH="${_qt_bin_dir}"
            "${WINDEPLOYQT_EXECUTABLE}" --pdb --no-angle "$<TARGET_FILE:Orbit>"
    COMMENT "Running windeployqt...")
endif()

strip_symbols(Orbit)