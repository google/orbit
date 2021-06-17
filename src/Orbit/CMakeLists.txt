# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(Orbit CXX)

add_executable(Orbit main.cpp)
target_compile_options(Orbit PRIVATE ${STRICT_COMPILE_FLAGS})
target_link_libraries(Orbit PRIVATE OrbitQt Style)
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

  # We have to wait for the OrbitQtTests to be finished building because
  # as part of this target also windeployqt is called which is also needed
  # by the Orbit target.
  # Calling windeployqt twice lead to problems in the past when both calls
  # were running simultaneously.
  add_dependencies(Orbit OrbitQtTests)
endif()

strip_symbols(Orbit)