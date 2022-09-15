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
    name = "ProcessService",
    deps = [
        "//src/GrpcProtos:module_cc_proto",
        "//src/GrpcProtos:process_cc_proto",
        "//src/GrpcProtos:services_cc_grpc_proto",
        "//src/GrpcProtos:services_cc_proto",
        "//src/GrpcProtos:tracepoint_cc_proto",
        "//src/ModuleUtils",
        "//src/ObjectUtils",
        "//src/OrbitBase",
        "//src/SymbolProvider",
        "//src/Symbols",
        "@com_github_grpc_grpc//:grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

orbit_cc_test(
    name = "ProcessServiceTests",
    deps = [
        ":ProcessService",
        "//src/GrpcProtos:services_cc_proto",
        "//src/GrpcProtos:tracepoint_cc_proto",
        "//src/OrbitBase",
        "//src/TestUtils",
        "@com_google_absl//absl/strings",
    ],
)
