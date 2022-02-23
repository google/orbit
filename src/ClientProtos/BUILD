# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

load("@rules_cc/cc:defs.bzl", "cc_proto_library")

package(default_visibility = [
    "//:__subpackages__",
    "//:users",  
])

licenses(["notice"])

# Proto Library targets

proto_library(
    name = "user_defined_capture_info_proto",
    srcs = ["user_defined_capture_info.proto"],
)

# CC Proto and GRPC Library targets

cc_proto_library(
    name = "user_defined_capture_info_cc_proto",
    deps = [
        ":user_defined_capture_info_proto",
    ],
)
