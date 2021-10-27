from conans import ConanFile, CMake, tools
import os


class LibunwindstackAndroidDependenciesConan(ConanFile):
    name = "libunwindstack-android-dependencies"
    version = "20210709"
    license = "MIT"
    author = "Henning Becker <henning.becker@gmail.com>"
    homepage = "https://android.googlesource.com/platform/system/libbase/"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    exports_sources = ["CMakeLists.txt", "cmake/FindFilesystem.cmake", "patches/*"]
    options = {"fPIC" : [True, False]}
    default_options = {"fPIC": True}

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def source(self):
        for source in self.conan_data["sources"][self.version]:
            tools.get(**source)
        for patch in self.conan_data.get("patches", {}).get(self.version, []):
            tools.patch(**patch)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        with open(os.path.join(self.package_folder, "LICENSE"), "w") as fd:
            for license in self.conan_data.get("license_files", {}).get(self.version, []):
                fd.write("================================================================================\n")
                fd.write("Name: {}\n".format(license["library_name"]))
                fd.write("URL: {}\n\n".format(license.get("library_url", "")))
                fd.write(open(os.path.join(self.source_folder, license["src"]), 'r').read())
                fd.write("\n\n")

            fd.write("================================================================================\n")

        self.copy("*.h", dst="include", src="libbase/include/")
        self.copy("*.h", dst="include", src="libprocinfo/include/")
        self.copy("*.h", dst="include", src="liblog/include/")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dll", dst="lib", keep_path=False)

    def package_info(self):
        # We can't list liblog here because we need to link it
        # dynamically in the libunwindstack tests and statically otherwise.
        # We compile both versions and include it in the package,
        # but won't enable automatic linking by listing it here.
        self.cpp_info.libs = ["procinfo", "base"]

