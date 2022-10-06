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
    name = "CaptureFile",
    deps = [
        "//src/ClientProtos:user_defined_capture_info_cc_proto",
        "//src/GrpcProtos:capture_cc_proto",
        "//src/OrbitBase",
        "@com_google_absl//absl/base",
        "@com_google_protobuf//:io_lite",
        "@com_google_protobuf//:protobuf",
    ],
)

orbit_cc_test(
    name = "CaptureFileTests",
    deps = [
        ":CaptureFile",
        "//src/GrpcProtos:capture_cc_proto",
        "//src/OrbitBase",
        "//src/TestUtils",
        "@com_google_absl//absl/base",
        "@com_google_protobuf//:io_lite",
    ],
)
