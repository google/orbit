# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


set(LLVM_VERSIONS 16 15 14 13 12)
foreach(VERSION ${LLVM_VERSIONS})
  find_package(LLVM CONFIG ${VERSION})
  if(LLVM_FOUND)
    break()
  endif()
endforeach()

if(NOT LLVM_FOUND)
  message(FATAL_ERROR "Could not find LLVM.")
endif()

add_library(LLVMHeaders INTERFACE IMPORTED)
target_include_directories(LLVMHeaders INTERFACE ${LLVM_INCLUDE_DIRS})

function(find_llvm_libs REQUIRED_LLVM_LIBS)
  set(LLVM_LIBS_TMP "")
  foreach(LIB IN LISTS REQUIRED_LLVM_LIBS)
    find_library(LLVM_LIB ${LIB} PATHS ${LLVM_LIBRARY_DIR})
    if(LLVM_LIB OR TARGET ${LIB})
      message(VERBOSE "Adding ${LIB}")
      list(APPEND LLVM_LIBS_TMP ${LIB})
    else()
      message(FATAL_ERROR "Can't find necessary LLVM library: ${LIB}")
    endif()
  endforeach()

  set(LLVM_LIBS ${LLVM_LIBS_TMP} PARENT_SCOPE)
  message(STATUS "Using LLVM libraries: ${LLVM_LIBS}")
endfunction()

# Call find_llvm_libs with the required LLVM libraries
find_llvm_libs("llvm_core llvm_irreader")
