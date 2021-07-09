from conans import ConanFile, CMake, tools
import os


class LibunwindstackConan(ConanFile):
    name = "libunwindstack"
    version = "20210709"
    license = "MIT"
    author = "Henning Becker <henning.becker@gmail.com>"
    homepage = "https://android.googlesource.com/platform/system/unwinding/"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    exports_sources = ["CMakeLists.txt", "cmake/FindFilesystem.cmake", "patches/*"]
    requires = ["lzma_sdk/19.00@orbitdeps/stable"]
    options = {"fPIC" : [True, False], "run_tests": [True, False]}
    default_options = {"fPIC": True, "run_tests": False}

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def build_requirements(self):
        self.build_requires("gtest/1.11.0", force_host_context=True)

    def configure(self):
        self.options["gtest"].no_main = False

    def source(self):
        for source in self.conan_data["sources"][self.version]:
            tools.get(**source)
        for patch in self.conan_data.get("patches", {}).get(self.version, []):
            tools.patch(**patch)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        if self.options.run_tests and not tools.cross_building(self, skip_x64_x86=True):
            cmake.test(output_on_failure=True)

    def package(self):
        with open(os.path.join(self.package_folder, "LICENSE"), "w") as fd:
            for license in self.conan_data.get("license_files", {}).get(self.version, []):
                fd.write("================================================================================\n")
                fd.write("Name: {}\n".format(license["library_name"]))
                fd.write("URL: {}\n\n".format(license.get("library_url", "")))
                fd.write(open(os.path.join(self.source_folder, license["src"]), 'r').read())
                fd.write("\n\n")

            fd.write("================================================================================\n")

        self.copy("*.h", dst="include", src="libunwindstack/include/")
        self.copy("*.h", dst="include", src="libbase/include/")
        self.copy("*.h", dst="include", src="libprocinfo/include/")
        self.copy("*unwindstack.lib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["unwindstack"]

    def package_id(self):
        # The resulting package is not affected by tests been run or not.
        del self.info.options.run_tests

