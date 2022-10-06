# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(LZMA STATIC)

set(DIR "${CMAKE_SOURCE_DIR}/third_party/lzma1900/C")
target_include_directories(LZMA SYSTEM PUBLIC "${DIR}")
target_compile_definitions(LZMA PUBLIC -D_7ZIP_ST)
target_sources(LZMA PRIVATE
  ${DIR}/7zAlloc.c
  ${DIR}/7zArcIn.c
  ${DIR}/7zBuf2.c
  ${DIR}/7zBuf.c
  ${DIR}/7zCrc.c
  ${DIR}/7zCrcOpt.c
  ${DIR}/7zDec.c
  ${DIR}/7zFile.c
  ${DIR}/7zStream.c
  ${DIR}/Aes.c
  ${DIR}/AesOpt.c
  ${DIR}/Alloc.c
  ${DIR}/Bcj2.c
  # ${DIR}/Bcj2Enc.c
  ${DIR}/Bra86.c
  ${DIR}/Bra.c
  ${DIR}/BraIA64.c
  ${DIR}/CpuArch.c
  ${DIR}/Delta.c
  # ${DIR}/DllSecur.c
  ${DIR}/LzFind.c
  # ${DIR}/LzFindMt.c
  ${DIR}/Lzma2Dec.c
  # ${DIR}/Lzma2DecMt.c
  ${DIR}/Lzma2Enc.c
  ${DIR}/Lzma86Dec.c
  ${DIR}/Lzma86Enc.c
  ${DIR}/LzmaDec.c
  ${DIR}/LzmaEnc.c
  ${DIR}/LzmaLib.c
  # ${DIR}/MtCoder.c
  # ${DIR}/MtDec.c
  ${DIR}/Ppmd7.c
  ${DIR}/Ppmd7Dec.c
  ${DIR}/Ppmd7Enc.c
  ${DIR}/Sha256.c
  ${DIR}/Sort.c
  # ${DIR}/Threads.c
  ${DIR}/Xz.c
  ${DIR}/XzCrc64.c
  ${DIR}/XzCrc64Opt.c
  ${DIR}/XzDec.c
  ${DIR}/XzEnc.c
  ${DIR}/XzIn.c
)

if(NOT MSVC)
  target_compile_options(LZMA PRIVATE
    -Wno-error=misleading-indentation
  )
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12)
  target_compile_options(LZMA PRIVATE
    -Wno-error=dangling-pointer
  )
endif()

add_library(LZMA::LZMA ALIAS LZMA)
