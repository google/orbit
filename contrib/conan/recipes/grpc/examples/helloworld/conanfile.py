# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from conans import ConanFile, CMake, tools
import os


class HelloWorldConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    name = "helloworld"
    version = "0.0.1"

    requires = "grpc/1.27.3@orbitdeps/stable"
    build_requires = "grpc_codegen/1.27.3@orbitdeps/stable"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
