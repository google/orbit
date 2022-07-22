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
    name = "LinuxTracing",
    deps = [
        "//src/ApiInterface",
        "//src/GrpcProtos",
        "//src/GrpcProtos:capture_cc_proto",
        "//src/GrpcProtos:module_cc_proto",
        "//src/GrpcProtos:tracepoint_cc_proto",
        "//src/Introspection",
        "//src/ModuleUtils",
        "//src/ObjectUtils",
        "//src/OrbitBase",
        "//third_party/libunwindstack",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/hash",
        "@com_google_absl//absl/meta:type_traits",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/synchronization",
    ],
)

orbit_cc_test(
    name = "LinuxTracingTests",
    deps = [
        ":LinuxTracing",
        "//src/GrpcProtos:capture_cc_proto",
        "//src/ModuleUtils",
        "//src/OrbitBase",
        "//third_party/libunwindstack",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/synchronization",
        "@com_google_protobuf//:protobuf",
    ],
)
