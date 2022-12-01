# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(Orbit CXX)

add_executable(Orbit)

target_sources(
    Orbit
    PRIVATE main.cpp
    ../../icons/orbiticons.qrc
)

target_link_libraries(Orbit PRIVATE
        ClientFlags
        CommandLineUtils
        OrbitQt
        Style
        absl::flags
        absl::flags_parse
        absl::flags_usage)
target_include_directories(Orbit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(Orbit PROPERTIES AUTORCC ON)

if(WIN32)
  set_target_properties(Orbit PROPERTIES WIN32_EXECUTABLE ON)

  include(${CMAKE_SOURCE_DIR}/cmake/version.cmake)
  GenerateVersionFile("${CMAKE_CURRENT_BINARY_DIR}/Orbit.rc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/Orbit.rc.in" Orbit)
  target_sources(Orbit PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/Orbit.rc)

  # We have to wait for the OrbitQtTests to be finished building because
  # as part of this target also windeployqt is called which is also needed
  # by the Orbit target.
  # Calling windeployqt twice lead to problems in the past when both calls
  # were running simultaneously.
  add_dependencies(Orbit OrbitQtTests)
endif()

if (NOT WIN32)
install(TARGETS Orbit
        CONFIGURATIONS Release
        RUNTIME DESTINATION /opt/orbitprofiler)
endif()
