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
    generators = ["cmake_multi"]
    options = {"system_qt": [True, False], "with_gui": [True, False],
               "fPIC": [True, False],
               "run_tests": [True, False],
               "build_target": "ANY"}
    default_options = {"system_qt": True, "with_gui": True,
                       "fPIC": True,
                       "run_tests": True,
                       "build_target": None}
    _orbit_channel = "orbitdeps/stable"
    exports_sources = "CMakeLists.txt", "Orbit*", "bin/*", "cmake/*", "third_party/*", "LICENSE"

    def _version(self):
        if not self.version:
            buf = StringIO()
            self.run("git describe --always --dirty", output=buf)
            self.version = buf.getvalue().strip()
            if self.version[0] == 'v':
                self.version = self.version[1:]

        return self.version

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def build_requirements(self):
        self.build_requires('protoc_installer/3.9.1@bincrafters/stable#0')
        self.build_requires('grpc_codegen/1.27.3@{}'.format(self._orbit_channel))
        self.build_requires('gtest/1.11.0', force_host_context=True)

    def requirements(self):
        self.requires("abseil/20211102.0")
        self.requires("bzip2/1.0.8", override=True)
        self.requires("capstone/4.0.2")
        self.requires("grpc/1.27.3@{}".format(self._orbit_channel))
        self.requires("c-ares/1.15.0", override=True)
        self.requires("llvm-core/12.0.0@{}".format(self._orbit_channel))
        self.requires("openssl/1.1.1k", override=True)
        self.requires("outcome/2.2.0")
        if self.settings.os != "Windows":
            self.requires(
                "libunwindstack-android-dependencies/20210709@{}".format(self._orbit_channel))
            self.requires("volk/1.2.170")
            self.requires("vulkan-headers/1.1.114.0")
        self.requires("zlib/1.2.11#9e0c292b60ce77402bd9be60dd68266f", override=True)

        if self.options.with_gui:
            self.requires("freetype/2.10.0@bincrafters/stable#0")
            self.requires("glad/0.1.34")
            self.requires("imgui/1.85")
            self.requires("libpng/1.6.37@bincrafters/stable#0", override=True)
            self.requires("libssh2/1.9.0#df2b6034da12cc5cb68bd3c5c22601bf")

            if not self.options.system_qt:
                self.requires("qt/5.15.1@{}#e659e981368e4baba1a201b75ddb89b6".format(self._orbit_channel))


    def configure(self):
        if self.settings.os != "Windows" and not self.options.fPIC:
            raise ConanInvalidConfiguration(
                "We only support compiling with fPIC enabled!")

        if self.options.with_gui and self.settings.arch == "x86":
            raise ConanInvalidConfiguration(
                "We don't actively support building the UI for 32bit platforms. Please remove this check in conanfile.py if you still want to do so!")

        self.options["gtest"].no_main = True

        if self.settings.os != "Windows":
            self.options["llvm-core"].with_xml2 = False

        if self.options.with_gui:

            if not self.options.system_qt:
                self.options["qt"].qtwebengine = True
                self.options["qt"].qtwebchannel = True
                self.options["qt"].qtwebsockets = True
                self.options["qt"].shared = True
                self.options["qt"].with_sqlite3 = False
                self.options["qt"].with_mysql = False
                self.options["qt"].with_pq = False
                self.options["qt"].with_odbc = False
                self.options["qt"].with_sdl2 = False
                self.options["qt"].with_openal = False

                if self.settings.os == "Windows":
                    self.options["qt"].qttools = True
                    self.options["qt"].with_glib = False
                    self.options["qt"].with_harfbuzz = False
                    self.options["qt"].opengl = "dynamic"


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
        self.copy("*", src="bin/fonts", dst="bin/fonts", symlinks=True)
        self.copy("*", src="bin/shaders", dst="bin/shaders", symlinks=True)
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

        if not self.options.system_qt:
            orbit_executable = "Orbit.exe" if self.settings.os == "Windows" else "Orbit"
            self.run("windeployqt --pdb --no-angle {}".format(orbit_executable), cwd=os.path.join(self.package_folder, "bin"), run_environment=True)

            # Package Visual C++ and C Runtime
            vcvars = tools.vcvars_dict(self)
            if 'VCToolsRedistDir' in vcvars:
                arch = 'x64' if self.settings.arch == 'x86_64' else 'x86'
                src_path = os.path.join(vcvars['VCToolsRedistDir'], arch, 'Microsoft.VC142.CRT')
                self.copy("*.dll", src=src_path, dst="bin")


    def deploy(self):
        self.copy("*", src="bin", dst="bin")
