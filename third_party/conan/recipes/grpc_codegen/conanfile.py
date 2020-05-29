from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
import os
import time
import platform


class grpcConan(ConanFile):
    name = "grpc_codegen"
    version = "1.27.3"
    description = "Google's RPC library and framework."
    topics = ("conan", "grpc", "rpc")
    url = "https://github.com/inexorgame/conan-grpc"
    homepage = "https://github.com/grpc/grpc"
    license = "Apache-2.0"
    exports_sources = ["CMakeLists.txt"]
    generators = "cmake"
    short_paths = True  # Otherwise some folders go out of the 260 chars path length scope rapidly (on windows)

    settings = "os_build", "arch_build", "compiler", "arch"
    options = {
        "fPIC": [True, False],
    }
    default_options = {
        "fPIC": True,
    }

    _source_subfolder = "source_subfolder"
    _build_subfolder = "build_subfolder"

    build_requires = (
        "abseil/20190808@orbitdeps/stable",
        "zlib/1.2.11",
        "openssl/1.0.2t",
        "protobuf/3.9.1@bincrafters/stable",
        "c-ares/1.15.0@conan/stable"
    )

    requires = (
        "protoc_installer/3.9.1@bincrafters/stable"
    )

    def configure(self):
        if self.settings.os_build == "Windows" and self.settings.compiler == "Visual Studio":
            compiler_version = int(str(self.settings.compiler.version))
            if compiler_version < 14:
                raise ConanInvalidConfiguration("gRPC can only be built with Visual Studio 2015 or higher.")

        self.options["abseil"].cxx_standard = 17

    def source(self):
        tools.get(**self.conan_data["sources"][self.version])
        extracted_dir = "grpc-" + self.version
        if platform.system() == "Windows":
            time.sleep(8) # Work-around, see https://github.com/conan-io/conan/issues/5205
        os.rename(extracted_dir, self._source_subfolder)

        cmake_path = os.path.join(self._source_subfolder, "CMakeLists.txt")
        tools.replace_in_file(cmake_path, "absl::strings", "CONAN_PKG::abseil")
        tools.replace_in_file(cmake_path, "absl::optional", "CONAN_PKG::abseil")
        tools.replace_in_file(cmake_path, "absl::inlined_vector", "CONAN_PKG::abseil")

    def _configure_cmake(self):
        cmake = CMake(self)

        cmake.definitions['gRPC_BUILD_CODEGEN'] = "ON"
        cmake.definitions['gRPC_BUILD_CSHARP_EXT'] = "OFF"
        cmake.definitions['gRPC_BUILD_TESTS'] = "OFF"
        cmake.definitions['gRPC_INSTALL'] = "OFF"

        # tell grpc to use the find_package versions
        cmake.definitions['gRPC_CARES_PROVIDER'] = "package"
        cmake.definitions['gRPC_ZLIB_PROVIDER'] = "package"
        cmake.definitions['gRPC_SSL_PROVIDER'] = "package"
        cmake.definitions['gRPC_PROTOBUF_PROVIDER'] = "none"
        cmake.definitions['gRPC_ABSL_PROVIDER'] = "none"

        # Workaround for https://github.com/grpc/grpc/issues/11068
        cmake.definitions['gRPC_GFLAGS_PROVIDER'] = "none"
        cmake.definitions['gRPC_BENCHMARK_PROVIDER'] = "none"

        # Compilation on minGW GCC requires to set _WIN32_WINNTT to at least 0x600
        # https://github.com/grpc/grpc/blob/109c570727c3089fef655edcdd0dd02cc5958010/include/grpc/impl/codegen/port_platform.h#L44
        if self.settings.os_build == "Windows" and self.settings.compiler == "gcc":
            cmake.definitions["CMAKE_CXX_FLAGS"] = "-D_WIN32_WINNT=0x600"
            cmake.definitions["CMAKE_C_FLAGS"] = "-D_WIN32_WINNT=0x600"

        cmake.configure(build_folder=self._build_subfolder)
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        #cmake = self._configure_cmake()
        #cmake.install()

        self.copy(pattern="LICENSE", dst="licenses")
        self.copy("*", dst="bin", src=os.path.join(self._build_subfolder, "bin"))

    def package_info(self):
        self.env_info.PATH.append(os.path.join(self.package_folder, "bin"))

    def package_id(self):
        del self.info.settings.compiler
        del self.info.settings.arch
        self.info.include_build_settings()

