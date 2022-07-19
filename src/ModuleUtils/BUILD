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
    name = "ModuleUtils",
    deps = [
        "//src/GrpcProtos:module_cc_proto",
        "//src/ObjectUtils",
        "//src/OrbitBase",
        "@com_github_ned14_outcome//outcome",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

orbit_cc_test(
    name = "ModuleUtilsTests",
    deps = [
        ":ModuleUtils",
        "//src/GrpcProtos:module_cc_proto",
        "//src/ObjectUtils",
        "//src/OrbitBase",
        "//src/TestUtils",
        "@com_github_ned14_outcome//outcome",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)
