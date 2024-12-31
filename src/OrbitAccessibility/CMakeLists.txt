# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitAccessibility CXX)
add_library(OrbitAccessibility STATIC)

target_include_directories(OrbitAccessibility PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(OrbitAccessibility PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(
  OrbitAccessibility
  PUBLIC include/OrbitAccessibility/AccessibleInterface.h
         include/OrbitAccessibility/AccessibleInterfaceRegistry.h
         include/OrbitAccessibility/AccessibleObjectFake.h
         include/OrbitAccessibility/AccessibleWidgetBridge.h
         )

target_sources(
  OrbitAccessibility
  PRIVATE AccessibleInterface.cpp
          AccessibleInterfaceRegistry.cpp)

target_link_libraries(
  OrbitAccessibility
  PUBLIC OrbitBase)

add_executable(OrbitAccessibilityTests)

target_sources(OrbitAccessibilityTests PRIVATE)

target_sources(OrbitAccessibilityTests PRIVATE
               AccessibleInterfaceRegistryTest.cpp)

target_link_libraries(
  OrbitAccessibilityTests
  PRIVATE OrbitAccessibility
          GTest_Main)

register_test(OrbitAccessibilityTests)
