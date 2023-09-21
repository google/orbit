# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


find_package(LLVM CONFIG 16)

if(NOT LLVM_FOUND)
  find_package(LLVM CONFIG 15)
endif()

if(NOT LLVM_FOUND)
  find_package(LLVM CONFIG 14)
endif()

if(NOT LLVM_FOUND)
  find_package(LLVM CONFIG 13)
endif()

if(NOT LLVM_FOUND)
  find_package(LLVM CONFIG 12 REQUIRED)
endif()

add_library(LLVMHeaders INTERFACE IMPORTED)
target_include_directories(LLVMHeaders INTERFACE ${LLVM_INCLUDE_DIRS})

function(find_llvm_libs REQUIRED_LLVM_LIBS)
  find_library(LLVM_SHARED_LIB LLVM NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH PATHS ${LLVM_LIBRARY_DIR})
  set(LLVM_LIBS_TMP "" PARENT_SCOPE)
  foreach(LIB IN LISTS REQUIRED_LLVM_LIBS)
    find_library(LLVM_LIB ${LIB} NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH PATHS ${LLVM_LIBRARY_DIR})
    if(LLVM_LIB OR TARGET ${LIB})
      message(VERBOSE "Adding ${LIB}")
      list(APPEND LLVM_LIBS_TMP ${LIB})
    elseif(LLVM_SHARED_LIB)
      set(LLVM_LIBS ${LLVM_SHARED_LIB} PARENT_SCOPE)
      message(STATUS "Using shared LLVM library.")
      return()
    else()
      message(FATAL_ERROR "Can't find necessary LLVM libraries")
    endif()
  endforeach()
  
  set(LLVM_LIBS ${LLVM_LIBS_TMP} PARENT_SCOPE)
  message(STATUS "Using static LLVM libraries.")
endfunction()
