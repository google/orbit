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
    name = "ApiUtils",
    deps = [
        "//src/ApiInterface",
        "//src/GrpcProtos:capture_cc_proto",
        "//src/OrbitBase",
        "@com_google_absl//absl/base",
    ],
)

orbit_cc_test(
    name = "ApiUtilsTests",
    deps = [
        ":ApiUtils",
        "//src/GrpcProtos:capture_cc_proto",
        "@com_google_protobuf//:differencer",
    ],
)
