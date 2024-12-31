# coding=utf-8

"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""
from conan import ConanFile

class OrbitDeps(ConanFile):
    name = "orbit_deps"
    version = "0.1"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("abseil/20240116.2")
        self.requires("capstone/5.0.1")
        self.requires("grpc/1.67.1")
        self.requires("gtest/1.15.0")
        self.requires("outcome/2.2.9")
        self.requires("llvm-core/13.0.0")
        if self.settings.os != "Windows":
            self.requires("volk/1.3.268.0")
            self.requires("vulkan-headers/1.3.290.0")
            self.requires("vulkan-validationlayers/1.3.290.0")
        self.requires("zlib/1.3.1", override=True)
        self.requires("openssl/3.3.2", override=True)
        self.requires("libssh2/1.11.0")

    def build_requirements(self):
        self.tool_requires("grpc/1.67.1")
        self.tool_requires("protobuf/5.27.0")
        self.tool_requires("gtest/1.15.0")

    def layout(self):
        self.folders.build = "build"
        self.folders.generators = "build/generators"
