# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

load("//:orbit.bzl", "orbit_cc_library", "orbit_cc_test")

package(default_visibility = [
    "//:__subpackages__",
    "//:users",  
])

licenses(["notice"])

orbit_cc_library(
    name = "Symbols",
    deps = [
        "//src/GrpcProtos:module_cc_proto",
        "//src/GrpcProtos:symbol_cc_proto",
        "//src/Introspection",
        "//src/ObjectUtils",
        "//src/OrbitBase",
        "//src/SymbolProvider",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
        "@llvm-project//llvm:Object",
    ],
)

orbit_cc_test(
    name = "SymbolsTests",
    deps = [
        ":Symbols",
        "//src/GrpcProtos:module_cc_proto",
        "//src/GrpcProtos:symbol_cc_proto",
        "//src/ObjectUtils",
        "//src/OrbitBase",
        "//src/SymbolProvider",
        "//src/TestUtils",
        "@com_google_absl//absl/strings",
    ],
)