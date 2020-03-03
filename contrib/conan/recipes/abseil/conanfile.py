from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
from conans.tools import Version
import os

class AbseilConan(ConanFile):
    name = "abseil"
    version = "20190808"
    url = "https://github.com/bincrafters/conan-abseil"
    homepage = "https://github.com/abseil/abseil-cpp"
    description = "Abseil Common Libraries (C++) from Google"
    topics = ("abseil", "algorithm", "container", "debugging", "hash", "memory", "meta", "numeric", "string",
                "synchronization", "time", "types", "utility")
    license = "Apache-2.0"
    #exports_sources = ["CMakeLists.txt"]
    generators = "cmake"
    settings = "os", "arch", "compiler", "build_type"
    requires = "cctz/2.3"
    options = {"cxx_standard": [11, 14, 17], "fPIC" : [True, False]}
    default_options = {"cxx_standard": 11, "fPIC": True}
    short_paths = True
    _source_subfolder = "source_subfolder"
    _commits = {
        "20190808": "aa844899c937bde5d2b24f276b59997e5b668bde",
    }

    def source(self):
        tools.get(**self.conan_data["sources"][self.version])
        extracted_dir = "abseil-cpp-" + self._commits[self.version]
        os.rename(extracted_dir, self._source_subfolder)
        tools.replace_in_file("{}/CMakeLists.txt".format(self._source_subfolder),
                                                         "project(absl CXX)",
                                                         '''project(absl CXX)
                                                            include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
                                                            conan_basic_setup()''')

        # That's a fix from upstream which prevents compiling abseil on
        # Windows. Check out https://github.com/abseil/abseil-cpp/issues/364
        # for more details.
        tools.replace_in_file("{}/absl/copts/GENERATED_AbseilCopts.cmake".format(self._source_subfolder),
                              '"/O2"', '')
        tools.replace_in_file("{}/absl/copts/GENERATED_AbseilCopts.cmake".format(self._source_subfolder),
                              '"/Ob2"', '')

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.settings.os == "Windows" and \
           self.settings.compiler == "Visual Studio" and \
           Version(self.settings.compiler.version.value) < "14":
            raise ConanInvalidConfiguration("Abseil does not support MSVC < 14")

    def build(self):
        cmake = CMake(self)
        cmake.definitions["BUILD_TESTING"] = "Off"
        cmake.definitions["CMAKE_CXX_STANDARD"] = self.options.cxx_standard
        cmake.definitions["ABSL_CCTZ_TARGET"] = "CONAN_PKG::cctz"
        cmake.configure(source_folder=self._source_subfolder)
        cmake.build()

    def package(self):
        self.copy("LICENSE", dst="licenses", src=self._source_subfolder)
        self.copy("*.h", dst="include", src=self._source_subfolder)
        self.copy("*.inc", dst="include", src=self._source_subfolder)
        self.copy("*.a", dst="lib", src=".", keep_path=False)
        self.copy("*.lib", dst="lib", src=".", keep_path=False)
        self.copy("*.so", dst="lib", src=".", keep_path=False)
        self.copy("*.dll", dst="bin", src=".", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = [
            "absl_flags_parse",
            "absl_flags_usage_internal",
            "absl_flags",
            "absl_flags_registry",
            "absl_flags_handle",
            "absl_flags_config",
            "absl_flags_internal",
            "absl_flags_usage",
            "absl_hashtablez_sampler",
            "absl_failure_signal_handler",
            "absl_synchronization",
            "absl_examine_stack",
            "absl_stacktrace",
            "absl_symbolize",
            "absl_graphcycles_internal",
            "absl_random_seed_sequences",
            "absl_flags_marshalling",
            "absl_random_internal_distribution_test_util",
            "absl_time",
            "absl_debugging_internal",
            "absl_malloc_internal",
            "absl_random_internal_pool_urbg",
            "absl_strings",
            "absl_base",
            "absl_random_internal_randen",
            "absl_str_format_internal",
            "absl_time_zone",
            "absl_strings_internal",
            "absl_spinlock_wait",
            "absl_random_seed_gen_exception",
            "absl_random_internal_seed_material",
            "absl_random_internal_randen_slow",
            "absl_random_internal_randen_hwaes_impl",
            "absl_random_internal_randen_hwaes",
            "absl_int128",
            "absl_dynamic_annotations",
            "absl_demangle_internal",
            "absl_throw_delegate",
            "absl_scoped_set_env",
            "absl_raw_hash_set",
            "absl_random_distributions",
            "absl_log_severity",
            "absl_leak_check_disable",
            #"absl_leak_check",
            "absl_hash",
            "absl_civil_time",
            "absl_city"]


        if self.settings.os == "Linux":
            self.cpp_info.system_libs.append("pthread")
