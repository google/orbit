from conans import ConanFile, CMake, tools


class LzmasdkConan(ConanFile):
    name = "lzma_sdk"
    version = "cb0b018"
    license = "MIT"
    author = "Henning Becker <henning.becker@gmail.com>"
    homepage = "https://www.7-zip.org/sdk.html"
    description = "The LZMA SDK provides the documentation, samples, header files, libraries, and tools you need to develop applications that use LZMA compression."
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    exports_sources = "CMakeLists.txt"
    options = {"fPIC" : [True, False]}
    default_options = {"fPIC": True}

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def source(self):
        self.run("git clone https://android.googlesource.com/platform/external/lzma")
        self.run("git checkout --detach {}".format(self.version), cwd="lzma/")


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

