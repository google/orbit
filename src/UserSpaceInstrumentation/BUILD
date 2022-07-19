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
    name = "UserSpaceInstrumentation",
    exclude = [
        "OrbitUserSpaceInstrumentation.*",
        "UserSpaceInstrumentationTestLib.*",
    ],
    deps = [
        "//net/grpc:grpc++",
        "//src/CaptureEventProducer",
        "//src/ModuleUtils",
        "//src/ObjectUtils",
        "//src/OrbitBase",
        "//src/ProducerSideChannel",
        "@com_github_capstone-engine_capstone//capstone",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:span",
    ],
)

cc_binary(
    name = "libOrbitUserSpaceInstrumentation.so",
    srcs = [
        "OrbitUserSpaceInstrumentation.cpp",
        "OrbitUserSpaceInstrumentation.h",
    ],
    linkshared = 1,
    linkstatic = 1,
    deps = [
        ":UserSpaceInstrumentation",
        "//src/CaptureEventProducer",
        "//src/OrbitBase",
        "//src/ProducerSideChannel",
    ],
)

cc_binary(
    name = "libUserSpaceInstrumentationTestLib.so",
    srcs = [
        "UserSpaceInstrumentationTestLib.cpp",
        "UserSpaceInstrumentationTestLib.h",
    ],
    linkshared = 1,
    linkstatic = 1,
    deps = [
        ":UserSpaceInstrumentation",
        "//src/CaptureEventProducer",
        "//src/OrbitBase",
        "//src/ProducerSideChannel",
    ],
)

orbit_cc_test(
    name = "UserSpaceInstrumentationTests",
    srcs = select({
        # TrampolineTest requires AVX support
        "//tools/target_cpu:haswell": ["TrampolineTest.cpp"],
        "//conditions:default": [],
    }),
    data = [
        ":libOrbitUserSpaceInstrumentation.so",
        ":libUserSpaceInstrumentationTestLib.so",
    ],
    # TrampolineTest requires AVX and is added selectively depending on the target architecture.
    exclude = ["TrampolineTest.cpp"],
    deps = [
        ":UserSpaceInstrumentation",
        "//src/GrpcProtos:capture_cc_proto",
        "//src/ModuleUtils",
        "//src/ObjectUtils",
        "//src/OrbitBase",
        "//src/TestUtils",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
    ],
)
