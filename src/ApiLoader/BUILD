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
    name = "ApiLoader",
    deps = [
        "//net/grpc:grpc++",
        "//src/ApiUtils",
        "//src/GrpcProtos:capture_cc_proto",
        "//src/ModuleUtils",
        "//src/ObjectUtils",
        "//src/OrbitBase",
        "//src/UserSpaceInstrumentation",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
    ],
)

orbit_cc_test(
    name = "ApiLoaderTests",
    deps = [
        ":ApiLoader",
    ],
)
