cmake_minimum_required(VERSION 3.15)

project(lzma C)

add_library(lzma STATIC)

target_include_directories(lzma PUBLIC lzma/C/)
target_compile_definitions(lzma PUBLIC -D_7ZIP_ST)

target_sources(
  lzma
  PRIVATE lzma/C/7zAlloc.c
          lzma/C/7zArcIn.c
          lzma/C/7zBuf.c
          lzma/C/7zBuf2.c
          lzma/C/7zCrc.c
          lzma/C/7zCrcOpt.c
          lzma/C/7zDec.c
          lzma/C/7zFile.c
          lzma/C/7zStream.c
          lzma/C/Aes.c
          lzma/C/AesOpt.c
          lzma/C/Alloc.c
          lzma/C/Bcj2.c
          lzma/C/Bra86.c
          lzma/C/Bra.c
          lzma/C/BraIA64.c
          lzma/C/CpuArch.c
          lzma/C/Delta.c
          lzma/C/LzFind.c
          lzma/C/Lzma2Dec.c
          lzma/C/Lzma2Enc.c
          lzma/C/Lzma86Dec.c
          lzma/C/Lzma86Enc.c
          lzma/C/LzmaDec.c
          lzma/C/LzmaEnc.c
          lzma/C/LzmaLib.c
          lzma/C/Ppmd7.c
          lzma/C/Ppmd7Dec.c
          lzma/C/Ppmd7Enc.c
          lzma/C/Sha256.c
          lzma/C/Sort.c
          lzma/C/Xz.c
          lzma/C/XzCrc64.c
          lzma/C/XzCrc64Opt.c
          lzma/C/XzDec.c
          lzma/C/XzEnc.c
          lzma/C/XzIn.c)

add_library(lzma::lzma ALIAS lzma)