from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
import os
import shutil
import time
import platform


class grpcConan(ConanFile):
    name = "grpc"
    version = "1.27.3"
    description = "Google's RPC library and framework."
    topics = ("conan", "grpc", "rpc")
    url = "https://github.com/inexorgame/conan-grpc"
    homepage = "https://github.com/grpc/grpc"
    license = "Apache-2.0"
    exports_sources = ["CMakeLists.txt", "gRPCTargets-helpers.cmake"]
    generators = "cmake"
    short_paths = True  # Otherwise some folders go out of the 260 chars path length scope rapidly (on windows)

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "fPIC": [True, False],
    }
    default_options = {
        "fPIC": True,
    }

    _source_subfolder = "source_subfolder"
    _build_subfolder = "build_subfolder"

    requires = (
        "abseil/20190808@orbitdeps/stable",
        "zlib/1.2.11",
        "openssl/1.0.2t",
        "protobuf/3.9.1@bincrafters/stable",
        "c-ares/1.15.0@conan/stable"
    )

    build_requires = (
        "protoc_installer/3.9.1@bincrafters/stable",
        "grpc_codegen/{}@orbitdeps/stable".format(version),
    )

    def configure(self):
        if self.settings.os == "Windows" and self.settings.compiler == "Visual Studio":
            del self.options.fPIC
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
        tools.replace_in_file(cmake_path, "set(_gRPC_CPP_PLUGIN $<TARGET_FILE:grpc_cpp_plugin>)", "find_program(_gRPC_CPP_PLUGIN grpc_cpp_plugin)")
        tools.replace_in_file(cmake_path, "DEPENDS ${ABS_FIL} ${_gRPC_PROTOBUF_PROTOC} grpc_cpp_plugin", "DEPENDS ${ABS_FIL} ${_gRPC_PROTOBUF_PROTOC} ${_gRPC_CPP_PLUGIN}")

    _cmake = None
    def _configure_cmake(self):
        if self._cmake:
            return self._cmake

        cmake = CMake(self)

        cmake.definitions['gRPC_BUILD_CODEGEN'] = "ON"
        cmake.definitions['gRPC_BUILD_CSHARP_EXT'] = "OFF"
        cmake.definitions['gRPC_BUILD_TESTS'] = "OFF"
        cmake.definitions['gRPC_INSTALL'] = "ON"

        cmake.definitions["gRPC_BUILD_GRPC_CPP_PLUGIN"] = "OFF"
        cmake.definitions["gRPC_BUILD_GRPC_CSHARP_PLUGIN"] = "OFF"
        cmake.definitions["gRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN"] = "OFF"
        cmake.definitions["gRPC_BUILD_GRPC_PHP_PLUGIN"] = "OFF"
        cmake.definitions["gRPC_BUILD_GRPC_PYTHON_PLUGIN"] = "OFF"
        cmake.definitions["gRPC_BUILD_GRPC_RUBY_PLUGIN"] = "OFF"
        cmake.definitions["gRPC_BUILD_GRPC_NODE_PLUGIN"] = "OFF"

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
        if self.settings.os == "Windows" and self.settings.compiler == "gcc":
            cmake.definitions["CMAKE_CXX_FLAGS"] = "-D_WIN32_WINNT=0x600"
            cmake.definitions["CMAKE_C_FLAGS"] = "-D_WIN32_WINNT=0x600"

        cmake.configure(build_folder=self._build_subfolder)
        self._cmake = cmake
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

        shutil.rmtree(os.path.join(self.package_folder, "lib", "pkgconfig"))
        shutil.rmtree(os.path.join(self.package_folder, "lib", "cmake", "grpc", "modules"))

        self.copy("gRPCTargets-helpers.cmake", dst=os.path.join("lib", "cmake", "grpc"))
        self.copy("LICENSE*", src=self._source_subfolder, dst="licenses")

    def package_info(self):
        self.cpp_info.libs = [
            "grpc++_unsecure",
            "grpc++_reflection",
            "grpc++_error_details",
            "grpc++",
            "grpc_unsecure",
            "grpc_plugin_support",
            "grpc_cronet",
            "grpcpp_channelz",
            "grpc",
            "gpr",
            "address_sorting",
            "upb",
        ]


        if self.settings.compiler == "Visual Studio":
            self.cpp_info.system_libs += ["wsock32", "ws2_32"]

