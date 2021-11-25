# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

load("//:orbit.bzl", "orbit_cc_library", "orbit_cc_test")

package(default_visibility = ["//:__subpackages__"])

licenses(["notice"])

orbit_cc_library(
    name = "UserSpaceInstrumentation",
    exclude = [
        "OrbitUserSpaceInstrumentation.*",
        "UserSpaceInstrumentationTestLib.*",
    ],
    deps = [
        "//src/CaptureEventProducer",
        "//src/ObjectUtils",
        "//src/OrbitBase",
        "//src/ProducerSideChannel",
        "@com_github_capstone-engine_capstone//capstone",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
    ],
)

cc_library(
    name = "OrbitUserSpaceInstrumentation",
    srcs = [
        "OrbitUserSpaceInstrumentation.cpp",
        "OrbitUserSpaceInstrumentation.h",
    ],
    deps = [
        ":UserSpaceInstrumentation",
        "//src/CaptureEventProducer",
        "//src/OrbitBase",
        "//src/ProducerSideChannel",
    ],
)

cc_library(
    name = "UserSpaceInstrumentationTestLib",
    srcs = [
        "UserSpaceInstrumentationTestLib.cpp",
        "UserSpaceInstrumentationTestLib.h",
    ],
    deps = [
        ":UserSpaceInstrumentation",
        "//src/CaptureEventProducer",
        "//src/OrbitBase",
        "//src/ProducerSideChannel",
    ],
)

genrule(
    name = "UserSpaceInstrumentationTestLibRuntimeDependency",
    srcs = [
        ":UserSpaceInstrumentationTestLib",
    ],
    outs = [
        "libUserSpaceInstrumentationTestLib.so",
    ],
    cmd = """
    for file in $(execpaths :UserSpaceInstrumentationTestLib); do
      if [[ $$file == *.so ]]; then
        cp "$$file" "$@"
        break
      fi
    done
  """,
)

genrule(
    name = "OrbitUserSpaceInstrumentationRuntimeDependency",
    srcs = [
        ":OrbitUserSpaceInstrumentation",
    ],
    outs = [
        "libOrbitUserSpaceInstrumentation.so",
    ],
    cmd = """
    for file in $(execpaths :OrbitUserSpaceInstrumentation); do
      if [[ $$file == *.so ]]; then
        cp "$$file" "$@"
        break
      fi
    done
  """,
)

orbit_cc_test(
    name = "UserSpaceInstrumentationTests",
    srcs = select({
        # TrampolineTest requires AVX support
        "//tools/target_cpu:haswell": ["TrampolineTest.cpp"],
        "//conditions:default": [],
    }),
    data = [
        ":OrbitUserSpaceInstrumentationRuntimeDependency",
        ":UserSpaceInstrumentationTestLibRuntimeDependency",
    ],
    # TrampolineTest requires AVX and is added selectively depending on the target architecture.
    exclude = ["TrampolineTest.cpp"],
    deps = [
        ":UserSpaceInstrumentation",
        "//src/GrpcProtos:capture_cc_proto",
        "//src/ObjectUtils",
        "//src/OrbitBase",
        "//src/TestUtils",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
    ],
)
