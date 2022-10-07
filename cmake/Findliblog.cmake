# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set(SRC_DIR "${CMAKE_SOURCE_DIR}/third_party/liblog")

if(NOT TARGET libcutils_headers)
  add_library(libcutils_headers INTERFACE)
  target_include_directories(libcutils_headers SYSTEM INTERFACE ${SRC_DIR}/../libcutils/include/)
endif()


if(NOT TARGET libbase_headers)
  add_library(libbase_headers INTERFACE)
  target_include_directories(libbase_headers SYSTEM INTERFACE ${SRC_DIR}/../libbase/include/)
endif()

add_library(liblog_common OBJECT)
file(GLOB liblog_source_files ${SRC_DIR}/*.cpp)

# These excluded files are android-only.
list(FILTER liblog_source_files EXCLUDE REGEX "(pmsg|logd)_(reader|writer)\.cpp$")
list(FILTER liblog_source_files EXCLUDE REGEX "(event_tag_map|log_time)\.cpp$")

target_sources(liblog_common PRIVATE ${liblog_source_files} ${SRC_DIR}/../libbase/logging.cpp)
target_include_directories(liblog_common PUBLIC ${SRC_DIR}/include/)
target_link_libraries(liblog_common PUBLIC libbase_headers libcutils_headers)

if(NOT MSVC)
  target_compile_options(liblog_common PRIVATE
    -Wno-error=unused-function
    -Wno-error=unknown-pragmas
    -Wno-error=old-style-cast
    -Wno-error=format-nonliteral
  )
endif()

add_library(liblog_shared SHARED $<TARGET_OBJECTS:liblog_common>)
target_include_directories(liblog_shared SYSTEM PUBLIC ${SRC_DIR}/include/)

add_library(liblog_static STATIC $<TARGET_OBJECTS:liblog_common>)
target_include_directories(liblog_static SYSTEM PUBLIC ${SRC_DIR}/include/)