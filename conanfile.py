# coding=utf-8

"""
Copyright (c) 2020 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
import os
import shutil
from io import StringIO
import csv


class OrbitConan(ConanFile):
    name = "OrbitProfiler"
    license = "BSD-2-Clause"
    url = "https://github.com/pierricgimmig/orbitprofiler.git"
    description = "C/C++ Performance Profiler"
    settings = "os", "compiler", "build_type", "arch"
    generators = ["CMakeDeps"]
    options = {"with_gui": [True, False],
               "fPIC": [True, False],
               "run_tests": [True, False],
               "build_target": "ANY",
               "with_system_deps": [True, False]}
    default_options = {"with_gui": True,
                       "fPIC": True,
                       "run_tests": True,
                       "build_target": None,
                       "with_system_deps": False}
    exports_sources = "CMakeLists.txt", "Orbit*", "bin/*", "cmake/*", "third_party/*", "LICENSE"

    def _version(self):
        if not self.version:
            buf = StringIO()
            self.run("git describe --always --dirty  --match 1.*", output=buf)
            self.version = buf.getvalue().strip()
            if self.version[0] == 'v':
                self.version = self.version[1:]

        return self.version

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def build_requirements(self):
        if self.options.with_system_deps: return
        self.build_requires('protobuf/3.21.4')
        self.build_requires('grpc/1.48.0')
        self.build_requires('gtest/1.11.0', force_host_context=True)

    def requirements(self):
        if self.options.with_system_deps: return

        self.requires("abseil/20220623.0")
        self.requires("capstone/4.0.2")
        self.requires("grpc/1.48.0")
        self.requires("outcome/2.2.3")
        self.requires("llvm-core/13.0.0")
        if self.settings.os != "Windows":
            self.requires("volk/1.3.224.1")
            self.requires("vulkan-headers/1.3.224.1")
            self.requires("vulkan-validationlayers/1.3.224.1")
        self.requires("zlib/1.2.12", override=True)
        self.requires("openssl/1.1.1s", override=True)

        if self.options.with_gui:
            self.requires("libssh2/1.10.0")


    def configure(self):
        if self.settings.os != "Windows" and not self.options.fPIC:
            raise ConanInvalidConfiguration(
                "We only support compiling with fPIC enabled!")

        if self.options.with_gui and self.settings.arch == "x86":
            raise ConanInvalidConfiguration(
                "We don't actively support building the UI for 32bit platforms. Please remove this check in conanfile.py if you still want to do so!")

        self.options["gtest"].no_main = True

        if self.settings.os != "Windows":
            self.options["vulkan-validationlayers"].with_wsi_xcb = False
            self.options["vulkan-validationlayers"].with_wsi_xlib = False
            self.options["vulkan-validationlayers"].with_wsi_wayland = False


    def build(self):
        cmake = CMake(self)
        cmake.definitions["WITH_GUI"] = "ON" if self.options.with_gui else "OFF"
        cmake.configure()
        cmake.build(target=str(self.options.build_target) if self.options.build_target else None)
        if self.options.run_tests and not tools.cross_building(self.settings, skip_x64_x86=True) and self.settings.get_safe("os.platform") != "GGP":
            cmake.test(output_on_failure=True)

    def imports(self):
        excludes = [
                "*qt*",
                "*licensewizard*",
                "*checklicenses*",
                "*.py",
                "*.pyc",
                "*.cc",
                "*.yml",
                "*.vanilla",
                "*.h",
                "*.pl",
                "*license_template.txt",
                "*.patch",
                "*.QT-LICENSE-AGREEMENT",
                "*.ini",
                "*.js",
                "*license-checker*",
                "*.html",
                "*.json",
                "*README.md"
        ]
        self.copy("LICENSE*", dst="licenses", folder=True, ignore_case=True, excludes=excludes)
        self.copy("LICENCE*", dst="licenses", folder=True, ignore_case=True, excludes=excludes)


    def package(self):
        self.copy("*", src="bin/autopresets", dst="bin/autopresets", symlinks=True)
        self.copy("*.pdb", src="bin/", dst="bin")
        self.copy("Orbit", src="bin/", dst="bin")
        self.copy("Orbit.exe", src="bin/", dst="bin")
        self.copy("Orbit.debug", src="bin/", dst="bin")
        self.copy("OrbitService", src="bin/", dst="bin")
        self.copy("OrbitService.exe", src="bin/", dst="bin")
        self.copy("OrbitService.debug", src="bin/", dst="bin")
        self.copy("OrbitClientGgp", src="bin/", dst="bin")
        self.copy("OrbitClientGgp.exe", src="bin/", dst="bin")
        self.copy("OrbitClientGgp.debug", src="bin/", dst="bin")
        self.copy("NOTICE")
        self.copy("LICENSE")
        self.copy("liborbit.so", src="lib/", dst="lib")
        self.copy("liborbituserspaceinstrumentation.so", src="lib/", dst="lib")
        self.copy("libOrbitVulkanLayer.so", src="lib/", dst="lib")
        self.copy("VkLayer_Orbit_implicit.json", src="lib/", dst="lib")
        self.copy("LinuxTracingIntegrationTests", src="bin/", dst="bin")
        self.copy("LinuxTracingIntegrationTests.debug", src="bin/", dst="bin")
        self.copy("OrbitServiceIntegrationTests", src="bin/", dst="bin")
        self.copy("OrbitServiceIntegrationTests.debug", src="bin/", dst="bin")
        self.copy("libIntegrationTestPuppetSharedObject.so", src="lib/", dst="lib")
        self.copy("libIntegrationTestPuppetSharedObject.so.debug", src="lib/", dst="lib")
        self.copy("OrbitFakeClient", src="bin/", dst="bin")
        self.copy("OrbitFakeClient.debug", src="bin/", dst="bin")
        self.copy("msdia140.dll", src="bin/", dst="bin")


    def deploy(self):
        self.copy("*", src="bin", dst="bin")
