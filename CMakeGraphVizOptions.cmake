# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Clean up stuff in OrbitQt
set (IGNORE_OTHER_EXE "OrbitTest.*;.*Fuzzer;.*Tests;")
set (IGNORE_CONAN_LIB "CONAN_LIB::.*;")
set (IGNORE_GTEST "GTest;::gtest;")
set (IGNORE_QT "Qt5::;")
set (IGNORE_WEBUI "WebUI;QWebChannelExtractor;")
set (IGNORE_INNER_LLVM "::llvm_headers;::llvm_binary_format;::llvm_support;::llvm_demangle;::llvm_bit_reader;::llvm_bitstream_reader;::llvm_core;::llvm_remarks;::llvm_mc;::llvm_debuginfo_codeview;::llvm_debuginfo_msf;::llvm_mc_parser;::llvm_textapi;::llvm_debuginfo_dwarf;::llvm_debuginfo_pdb;")

# Additional cleanup for OrbitService
set (IGNORE_CLIENT_GGP "OrbitClientGgp;OrbitCaptureGgpService;OrbitTriggerCaptureVulkanLayer;OrbitCaptureGgpClient")

string(CONCAT GRAPHVIZ_IGNORE_TARGETS 
    "${IGNORE_OTHER_EXE}"
    "${IGNORE_CONAN_LIB}"
    "${IGNORE_GTEST}"
    "${IGNORE_QT}"
    "${IGNORE_WEBUI}"
    "${IGNORE_INNER_LLVM}"
    "${IGNORE_CLIENT_GGP}")

set(GRAPHVIZ_GENERATE_PER_TARGET FALSE)
set(GRAPHVIZ_GENERATE_DEPENDERS FALSE)

