# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(MoveFilesToDocuments CXX)

add_library(MoveFilesToDocuments OBJECT)

target_compile_options(MoveFilesToDocuments PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(MoveFilesToDocuments PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_sources(MoveFilesToDocuments PUBLIC
        include/MoveFilesToDocuments/MoveFilesToDocuments.h)

target_sources(MoveFilesToDocuments PRIVATE
        MoveFilesDialog.cpp
        MoveFilesDialog.h
        MoveFilesDialog.ui
        MoveFilesProcess.cpp
        MoveFilesProcess.h
        MoveFilesToDocuments.cpp)

target_link_libraries(MoveFilesToDocuments PUBLIC
        OrbitBase
        OrbitCore
        Qt5::Widgets
        Qt5::Core
        CONAN_PKG::abseil)

set_target_properties(MoveFilesToDocuments PROPERTIES AUTOMOC ON)
set_target_properties(MoveFilesToDocuments PROPERTIES AUTOUIC ON)
set_target_properties(MoveFilesToDocuments PROPERTIES AUTORCC ON)
iwyu_add_dependency(MoveFilesToDocuments)
