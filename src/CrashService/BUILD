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
    name = "CrashService",
    deps = [
        "//src/GrpcProtos:services_cc_grpc_proto",
        "//src/GrpcProtos:services_cc_proto",
        "//src/OrbitBase",
        "@com_github_grpc_grpc//:grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

orbit_cc_test(
    name = "CrashServiceTest",
    deps = [
        ":CrashService",
    ],
)
