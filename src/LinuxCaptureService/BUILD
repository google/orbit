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
    name = "LinuxCaptureService",
    deps = [
        "//src/ApiLoader",
        "//src/ApiUtils",
        "//src/CaptureServiceBase",
        "//src/GrpcProtos",
        "//src/GrpcProtos:capture_cc_proto",
        "//src/Introspection",
        "//src/LinuxTracing",
        "//src/MemoryTracing",
        "//src/ObjectUtils",
        "//src/OrbitBase",
        "//src/OrbitVersion",
        "//src/ProducerEventProcessor",
        "//src/UserSpaceInstrumentation",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
    ],
)

orbit_cc_test(
    name = "LinuxCaptureServiceTest",
    deps = [
        ":LinuxCaptureService",
        "//src/TestUtils",
    ],
)
