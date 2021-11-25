# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

load("//:orbit.bzl", "orbit_cc_library", "orbit_cc_test")

package(default_visibility = ["//:__subpackages__"])

licenses(["notice"])

orbit_cc_library(
    name = "FramePointerValidator",
    deps = [
        "//src/GrpcProtos:code_block_cc_proto",
        "//src/OrbitBase",
        "@com_github_capstone-engine_capstone//capstone",
    ],
)

orbit_cc_test(
    name = "FramePointerValidatorTests",
    deps = [
        ":FramePointerValidator",
        "//src/GrpcProtos:code_block_cc_proto",
        "//src/GrpcProtos:symbol_cc_proto",
        "//src/ObjectUtils",
        "//src/OrbitBase",
        "@com_github_capstone-engine_capstone//capstone",
    ],
)
