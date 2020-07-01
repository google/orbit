from conans import ConanFile, CMake, tools


class LibprotobufMutatorConan(ConanFile):
    name = "libprotobuf-mutator"
    version = "20200506"
    license = "Apache-2.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    exports_sources = "patches/*",
    build_requires = "protoc_installer/3.9.1@bincrafters/stable",
    options = { "fPIC" : [True, False] }
    default_options = { "fPIC" : True }

    def configure(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def source(self):
        tools.get(**self.conan_data["sources"][self.version])
        for patch in self.conan_data["patches"][self.version]:
            tools.patch(**patch)

    def requirements(self):
        self.requires("lzma_sdk/19.00@orbitdeps/stable")
        self.requires("zlib/1.2.11")
        self.requires("protobuf/3.9.1@bincrafters/stable")

    def build(self):
        self._source_subfolder = self.conan_data["source_subfolder"][self.version]
        cmake = CMake(self)
        cmake.definitions["LIB_PROTO_MUTATOR_TESTING"] = False
        cmake.definitions["CMAKE_CXX_FLAGS"] = "-fPIE"
        cmake.definitions["CMAKE_C_FLAGS"] = "-fPIE"
        cmake.configure(source_folder=self._source_subfolder)
        cmake.build()

    def package(self):
        self.copy("*.h", dst="include",
                  src="{}/src".format(self._source_subfolder))
        self.copy("*.h", dst="include/port",
                  src="{}/port".format(self._source_subfolder))
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.pdb", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.libs = ["protobuf-mutator-libfuzzer", "protobuf-mutator"]
