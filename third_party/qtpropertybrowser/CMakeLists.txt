# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(qtpropertybrowser STATIC)

target_include_directories(qtpropertybrowser SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(qtpropertybrowser PUBLIC Qt5::Widgets)
set_target_properties(qtpropertybrowser PROPERTIES AUTOMOC ON)
set_target_properties(qtpropertybrowser PROPERTIES AUTORCC ON)


target_sources(qtpropertybrowser PRIVATE
    qtbuttonpropertybrowser.h
    qteditorfactory.h
    qtgroupboxpropertybrowser.h
    qtpropertybrowser.h
    qtpropertybrowserutils_p.h
    qtpropertymanager.h
    qttreepropertybrowser.h
    qtvariantproperty.h
    qtbuttonpropertybrowser.cpp
    qteditorfactory.cpp
    qtgroupboxpropertybrowser.cpp
    qtpropertybrowser.cpp
    qtpropertybrowserutils.cpp
    qtpropertymanager.cpp
    qttreepropertybrowser.cpp
    qtvariantproperty.cpp
    qtpropertybrowser.qrc)


add_library(qtpropertybrowser::qtpropertybrowser ALIAS qtpropertybrowser)
