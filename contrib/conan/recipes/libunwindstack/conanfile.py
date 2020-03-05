from conans import ConanFile, CMake, tools


class LibunwindstackConan(ConanFile):
    name = "libunwindstack"
    version = "80a734f14"
    license = "MIT"
    author = "Henning Becker <henning.becker@gmail.com>"
    homepage = "https://android.googlesource.com/platform/system/core/+/master/libunwindstack/"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    exports_sources = ["CMakeLists.txt", "overrides/file.cpp"]
    requires = ["lzma_sdk/cb0b018@orbitdeps/stable"]

    def source(self):
        self.run("git clone https://android.googlesource.com/platform/system/core.git android-core")
        self.run("git checkout --detach {}".format(self.version), cwd="android-core/")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("*.h", dst="include", src="android-core/libunwindstack/include/")
        self.copy("*.h", dst="include", src="android-core/base/include/")
        self.copy("*unwindstack.lib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["unwindstack"]

