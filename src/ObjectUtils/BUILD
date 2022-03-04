# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

load("//:orbit.bzl", "orbit_cc_library", "orbit_cc_test")

package(default_visibility = [
    "//:__subpackages__",
    "//:users",
])

licenses(["notice"])

orbit_cc_library(
    name = "ObjectUtils",
    exclude = glob(["*Dia.*"]),
    deps = [
        "//src/GrpcProtos:module_cc_proto",
        "//src/GrpcProtos:symbol_cc_proto",
        "//src/OrbitBase",
        "//src/Introspection",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@llvm-project//llvm:BinaryFormat",
        "@llvm-project//llvm:DebugInfoCodeView",
        "@llvm-project//llvm:DebugInfoDWARF",
        "@llvm-project//llvm:DebugInfoMSF",
        "@llvm-project//llvm:DebugInfoPDB",
        "@llvm-project//llvm:Demangle",
        "@llvm-project//llvm:Object",
        "@llvm-project//llvm:Support",
        "@llvm-project//llvm:Symbolize",
    ],
)

orbit_cc_test(
    name = "ObjectUtilsTests",
    exclude = glob(["*DiaTest.*"]),
    deps = [
        ":ObjectUtils",
        "//src/GrpcProtos:module_cc_proto",
        "//src/GrpcProtos:symbol_cc_proto",
        "//src/OrbitBase",
        "//src/TestUtils",
        "@com_github_ned14_outcome//outcome",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:span",
    ],
)
