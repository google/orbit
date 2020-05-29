from conans import ConanFile, CMake, tools
import shutil


class LibDisasmConan(ConanFile):
    name = "libdisasm"
    version = "0.23"
    license = "Clarified Artistic License"
    description = "An basic x86 disassembler in library form."
    topics = ("libdisasm", "disasm")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}
    generators = "cmake"
    exports_sources = [ "CMakeLists.txt", "sizeofvoid.patch" ]

    def source(self):
        tools.download("https://sourceforge.net/projects/bastard/files/libdisasm/{0}/libdisasm-{0}.tar.gz/download".format(self.version),
                      "libdisasm-{}.tar.gz".format(self.version))
        tools.untargz("libdisasm-{}.tar.gz".format(self.version))
        tools.patch(patch_file="sizeofvoid.patch",
                    base_path="libdisasm-{}".format(self.version))
        shutil.move("CMakeLists.txt", "libdisasm-{}/".format(self.version))

    def get_env(self):
        cmake = CMake(self)
        cmake.configure(source_folder="libdisasm-{}".format(self.version))
        return cmake

    def build(self):
        cmake = self.get_env()
        cmake.build()

    def package(self):
        cmake = self.get_env()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["disasm"]
        self.cpp_info.includedirs = ["include"]

