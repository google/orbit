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
    name = "ProducerSideService",
    deps = [
        "//src/CaptureServiceBase",
        "//src/GrpcProtos",
        "//src/GrpcProtos:capture_cc_proto",
        "//src/GrpcProtos:producer_side_services_cc_proto",
        "//src/OrbitBase",
        "//src/ProducerEventProcessor",
        "//src/ProducerSideChannel",
        "@com_github_grpc_grpc//:grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_protobuf//:arena",
    ],
)

orbit_cc_test(
    name = "ProducerSideServiceTests",
    deps = [
        ":ProducerSideService",
        "//src/GrpcProtos",
        "//src/GrpcProtos:capture_cc_proto",
        "//src/GrpcProtos:producer_side_services_cc_grpc_proto",
        "//src/GrpcProtos:producer_side_services_cc_proto",
        "//src/OrbitBase",
        "//src/ProducerEventProcessor",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/synchronization",
        "@com_google_protobuf//:differencer",
    ],
)
