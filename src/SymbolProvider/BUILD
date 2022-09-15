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
    name = "SymbolProvider",
    deps = [
        "//src/OrbitBase",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format"
    ],
)

orbit_cc_test(
    name = "SymbolProviderTests",
    deps = [
        ":SymbolProvider",
        "//src/OrbitBase",
        "//src/TestUtils",
        "@com_google_absl//absl/hash:hash_testing_no_google3",
        "@com_google_absl//absl/strings",
    ],
)