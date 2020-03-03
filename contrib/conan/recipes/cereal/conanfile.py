#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
from conans import ConanFile, CMake, tools


class CerealConan(ConanFile):
    name = "cereal"
    description = "Serialization header-only library for C++11."
    version = "1.3.0"
    url = "https://github.com/conan-community/conan-cereal"
    homepage = "https://github.com/USCiLab/cereal"
    author = "Conan Community <info@conan.io>"
    topics = ("conan", "cereal", "header-only", "serialization", "cpp11")
    license = "BSD-3"
    options = {"thread_safe": [True, False]}
    default_options = {"thread_safe": False}
    exports = "LICENSE"
    exports_sources = "CMakeLists.txt"
    generators = "cmake"
    no_copy_source = True

    def get_source_folder(self):
        return "{}-{}".format(self.name, self.version)

    def source(self):
        source_url = ("%s/archive/v%s.zip" % (self.homepage, self.version))
        tools.get(source_url)

    def package(self):
        self.copy("LICENSE", dst="licenses", src=self.get_source_folder())
        cmake = CMake(self)
        cmake.definitions["JUST_INSTALL_CEREAL"] = True
        cmake.configure(source_folder=self.get_source_folder())
        cmake.install()

    def package_id(self):
        self.info.header_only()

    def package_info(self):
        if self.options.thread_safe:
            self.cpp_info.defines = ["CEREAL_THREAD_SAFE=1"]
            if tools.os_info.is_linux:
                self.cpp_info.libs.append("pthread")
        self.cpp_info.includedirs = [ "include" ]
