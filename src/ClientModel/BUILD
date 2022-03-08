# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

package(default_visibility = [
    "//:__subpackages__",
    "//:users",  
])

licenses(["notice"])

cc_library(
    name = "capture_serializer",
    srcs = ["CaptureSerializer.cpp"],
    hdrs = ["include/ClientModel/CaptureSerializer.h"],
    compatible_with = ["//buildenv/target:gce"],
    includes = ["include/"],
    deps = [
        "@com_google_net//net/proto2/io/public:io_lite",
        "@com_google_net//net/proto2/public",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "//src/ClientProtos:capture_data_cc_proto",
        "//src/GrpcProtos:module_cc_proto",
        "//src/OrbitBase",
    ],
)