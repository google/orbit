from conans import ConanFile, CMake, tools
import os


class LzmasdkConan(ConanFile):
    name = "lzma_sdk"
    version = "19.00"
    license = "MIT"
    author = "Henning Becker <henning.becker@gmail.com>"
    homepage = "https://www.7-zip.org/sdk.html"
    description = "The LZMA SDK provides the documentation, samples, header files, libraries, and tools you need to develop applications that use LZMA compression."
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    exports_sources = "CMakeLists.txt"
    options = {"fPIC" : [True, False]}
    default_options = {"fPIC": True}
    build_requires = "cmake/[>3.15]"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def source(self):
        tools.download("https://www.7-zip.org/a/lzma1900.7z", sha256='00f569e624b3d9ed89cf8d40136662c4c5207eaceb92a70b1044c77f84234bad', filename='lzma.7z')
        os.mkdir("lzma")
        with tools.chdir("./lzma/"):
            self.run("cmake -E tar x ../lzma.7z")
        os.remove("lzma.7z")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("*.h", dst="include", src="lzma/C")
        self.copy("*lzma_sdk.lib", dst="lib", keep_path=False)
        self.copy("*.pdb", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["lzma_sdk"]
        self.cpp_info.defines = ["_7ZIP_ST"]

