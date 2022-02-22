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
    name = "CaptureEventProducer",
    deps = [
        "//src/GrpcProtos:capture_cc_proto",
        "//src/GrpcProtos:producer_side_services_cc_grpc_proto",
        "//src/GrpcProtos:producer_side_services_cc_proto",
        "//src/OrbitBase",
        "@com_github_cameron314_concurrentqueue//concurrentqueue",
        "@com_github_grpc_grpc//:grpc",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_protobuf//:arena",
    ],
)

orbit_cc_test(
    name = "CaptureEventProducerTests",
    deps = [
        ":CaptureEventProducer",
        "//src/FakeProducerSideService",
        "//src/GrpcProtos:capture_cc_proto",
        "//src/GrpcProtos:producer_side_services_cc_proto",
        "@com_github_grpc_grpc//:grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_protobuf//:arena",
    ],
)
