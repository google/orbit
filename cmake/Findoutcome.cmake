# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
find_package(outcome CONFIG)

if(outcome_FOUND)
  message("Found outcome as a distinct package - either through Conan or as a system library.")
  return()
endif()

message("Couldn't find outcome as a distinct package. Trying to use the version from Boost.")

find_package(Boost REQUIRED)

add_library(outcome INTERFACE)
target_include_directories(outcome INTERFACE ${CMAKE_SOURCE_DIR}/third_party/Outcome/include)
target_link_libraries(outcome INTERFACE Boost::headers)

add_library(outcome::outcome ALIAS outcome)
message("Found outcome in Boost!")