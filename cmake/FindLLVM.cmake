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
