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
    name = "Service",
    exclude = [
        "main.cpp",
    ],
    deps = [
        "//src/ApiLoader",
        "//src/ApiUtils",
        "//src/CaptureServiceBase",
        "//src/CrashService",
        "//src/GrpcProtos",
        "//src/GrpcProtos:capture_cc_proto",
        "//src/GrpcProtos:code_block_cc_proto",
        "//src/GrpcProtos:module_cc_proto",
        "//src/GrpcProtos:process_cc_proto",
        "//src/GrpcProtos:services_cc_grpc_proto",
        "//src/GrpcProtos:services_cc_proto",
        "//src/GrpcProtos:tracepoint_cc_proto",
        "//src/Introspection",
        "//src/LinuxCaptureService",
        "//src/LinuxTracing",
        "//src/MemoryTracing",
        "//src/ObjectUtils",
        "//src/OrbitBase",
        "//src/OrbitVersion",
        "//src/ProcessService",
        "//src/ProducerSideService",
        "//src/TracepointService",
        "//src/UserSpaceInstrumentation",
        "@com_github_grpc_grpc//:grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_protobuf//:arena",
    ],
)

cc_binary(
    name = "OrbitService",
    srcs = [
        "main.cpp",
    ],
    deps = [
        ":Service",
        "//src/OrbitBase",
        "//src/OrbitVersion",
        "@com_google_absl//absl/flags:config",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
    ],
)
