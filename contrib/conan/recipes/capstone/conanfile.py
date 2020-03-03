from conans import ConanFile, CMake, tools


class CapstoneConan(ConanFile):
    name = "capstone"
    version = "4.0.1"
    license = "BSD-3-Clause"
    description = "Capstone is a disassembler framework for almost every platform"
    topics = ("disassembler", "arm", "x86_64", "x86", "arm64", "bpf", "riscv",
             "mips")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}
    generators = "cmake"

    def source(self):
        self.run("git clone https://github.com/aquynh/capstone.git")
        self.run("git checkout {}".format(self.version), cwd="capstone/")
        # This small hack might be useful to guarantee proper /MT /MD linkage
        # in MSVC if the packaged project doesn't have variables to set it
        # properly
        tools.replace_in_file("capstone/CMakeLists.txt", "project(capstone)",
                              '''project(capstone)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()''')

    def _get_cmake(self):
        cmake = CMake(self)
        cmake.definitions["CAPSTONE_BUILD_STATIC_RUNTIME"] = True
        cmake.definitions["CAPSTONE_BUILD_STATIC"] = not self.options.shared
        cmake.definitions["CAPSTONE_BUILD_SHARED"] = self.options.shared
        cmake.definitions["CAPSTONE_BUILD_DIET"] = False
        cmake.definitions["CAPSTONE_BUILD_TESTS"] = False
        cmake.definitions["CAPSTONE_BUILD_CSTOOL"] = False
        cmake.definitions["CAPSTONE_BUILD_DIET"] = False
        cmake.definitions["CAPSTONE_ARM_SUPPORT"] = False
        cmake.definitions["CAPSTONE_ARM64_SUPPORT"] = False
        cmake.definitions["CAPSTONE_M68K_SUPPORT"] = False
        cmake.definitions["CAPSTONE_MIPS_SUPPORT"] = False
        cmake.definitions["CAPSTONE_PPC_SUPPORT"] = False
        cmake.definitions["CAPSTONE_SPARC_SUPPORT"] = False
        cmake.definitions["CAPSTONE_SYSZ_SUPPORT"] = False
        cmake.definitions["CAPSTONE_XCORE_SUPPORT"] = False
        cmake.definitions["CAPSTONE_X86_SUPPORT"] = True
        cmake.definitions["CAPSTONE_TMS320C64X_SUPPORT"] = False
        cmake.definitions["CAPSTONE_M680X_SUPPORT"] = False
        cmake.definitions["CAPSTONE_EVM_SUPPORT"] = False
        cmake.definitions["CAPSTONE_MOS65XX_SUPPORT"] = False
        cmake.configure(source_folder="capstone")
        return cmake

    def build(self):
        cmake = self._get_cmake()
        cmake.build()

    def package(self):
        cmake = self._get_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["capstone"]
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.libdirs = ["lib", "lib64"]

