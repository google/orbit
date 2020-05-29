from conans.model.conan_file import ConanFile
from conans import CMake
import os

############### CONFIGURE THESE VALUES ##################
default_user = "smspillaz"
default_channel = "ci"
#########################################################

channel = os.getenv("CONAN_CHANNEL", default_channel)
username = os.getenv("CONAN_USERNAME", default_user)


class DefaultNameConan(ConanFile):
    name = "DefaultName"
    version = "0.1"
    settings = "os", "compiler", "arch", "build_type"
    requires = "llvm/9.0.1@%s/%s" % (username, channel)
    generators = "cmake"

    def build(self):
        cmake = CMake(self)
        cmake.configure(defs={"CMAKE_VERBOSE_MAKEFILE":True,
            "CMAKE_EXPORT_COMPILE_COMMANDS": True})
        cmake.build()

    def imports(self):
        return
        self.copy(pattern="*.dll", dst="bin", src="bin")
        self.copy(pattern="*.dylib", dst="bin", src="lib")
        self.copy(pattern="clang", dst="bin", src="bin")

    def test(self):
        pass
