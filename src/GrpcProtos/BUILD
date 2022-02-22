# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

load("@rules_cc/cc:defs.bzl", "cc_proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")
load("//:orbit.bzl", "orbit_cc_library", "orbit_cc_test")

package(default_visibility = [
    "//:__subpackages__",
    "//:users",    
])

licenses(["notice"])

# This library target is needed because there is a header-file in include/.
orbit_cc_library(
    name = "GrpcProtos",
)

orbit_cc_test(
    name = "GrpcProtosTests",
    deps = [
        ":GrpcProtos",
    ],
)

# Proto Library targets

proto_library(
    name = "capture_proto",
    srcs = ["capture.proto"],
    deps = [
        ":module_proto",
        ":tracepoint_proto",
    ],
)

proto_library(
    name = "code_block_proto",
    srcs = ["code_block.proto"],
)

proto_library(
    name = "module_proto",
    srcs = ["module.proto"],
)

proto_library(
    name = "process_proto",
    srcs = ["process.proto"],
)

proto_library(
    name = "producer_side_services_proto",
    srcs = ["producer_side_services.proto"],
    has_services = True,
    deps = [
        ":capture_proto",
    ],
)

proto_library(
    name = "services_ggp_proto",
    srcs = ["services_ggp.proto"],
    has_services = True,
)

proto_library(
    name = "services_proto",
    srcs = ["services.proto"],
    has_services = True,
    deps = [
        ":capture_proto",
        ":code_block_proto",
        ":module_proto",
        ":process_proto",
        ":tracepoint_proto",
    ],
)

proto_library(
    name = "symbol_proto",
    srcs = ["symbol.proto"],
)

proto_library(
    name = "tracepoint_proto",
    srcs = ["tracepoint.proto"],
)

# CC Proto and GRPC Library targets

cc_proto_library(
    name = "code_block_cc_proto",
    deps = [
        ":code_block_proto",
    ],
)

cc_proto_library(
    name = "capture_cc_proto",
    deps = [
        ":capture_proto",
    ],
)

cc_proto_library(
    name = "module_cc_proto",
    deps = [
        ":module_proto",
    ],
)

cc_proto_library(
    name = "symbol_cc_proto",
    deps = [
        ":symbol_proto",
    ],
)

cc_proto_library(
    name = "tracepoint_cc_proto",
    deps = [
        ":tracepoint_proto",
    ],
)

cc_proto_library(
    name = "producer_side_services_cc_proto",
    deps = [
        ":producer_side_services_proto",
    ],
)

cc_proto_library(
    name = "process_cc_proto",
    deps = [
        ":process_proto",
    ],
)

cc_grpc_library(
    name = "producer_side_services_cc_grpc_proto",
    srcs = [
        ":producer_side_services_proto",
    ],
    deps = [
        ":producer_side_services_cc_proto",
    ],
)

cc_proto_library(
    name = "services_cc_proto",
    deps = [
        ":services_proto",
    ],
)

cc_grpc_library(
    name = "services_cc_grpc_proto",
    srcs = [
        ":services_proto",
    ],
    deps = [
        ":services_cc_proto",
    ],
)
