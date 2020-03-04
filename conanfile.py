from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
import os

class OrbitConan(ConanFile):
    name = "OrbitProfiler"
    version = "0.0.1"
    license = "BSD-2-Clause"
    url = "https://github.com/pierricgimmig/orbitprofiler.git"
    description = "C/C++ Performance Profiler"
    settings = "os", "compiler", "build_type", "arch"
    generators = [ "cmake_find_package_multi", "cmake" ]
    options = {"system_mesa": [True, False], "ggp": [True, False],
               "system_qt": [True, False], "with_gui": [True, False]}
    default_options = {"system_mesa": True, "ggp": False,
                       "system_qt": True, "with_gui": True}
    _orbit_channel = "orbitdeps/stable"
    exports_sources = "CMakeLists.txt", "Orbit*", "bin/*", "cmake/*", "external/*", "LICENSE"

    def requirements(self):
        if self.settings.os != "Windows" and self.options.with_gui and not self.options.system_qt and self.options.system_mesa:
            raise ConanInvalidConfiguration("When disabling system_qt, you also have to "
                                            "disable system mesa.")

        self.requires("asio/1.12.2@bincrafters/stable")
        self.requires("abseil/20190808@{}".format(self._orbit_channel))
        self.requires("bzip2/1.0.8@conan/stable")
        self.requires("capstone/4.0.1@{}".format(self._orbit_channel))
        self.requires("cereal/1.3.0@{}".format(self._orbit_channel))
        self.requires("gtest/1.8.1@bincrafters/stable")
        self.requires("libcurl/7.66.0")
        self.requires("llvm_object/9.0.1@orbitdeps/stable")
        self.requires("openssl/1.1.1d@{}".format(self._orbit_channel))
        self.requires("zlib/1.2.11@conan/stable")

        if self.settings.os == "Windows":
            self.requires("breakpad/216cea7b@{}".format(self._orbit_channel))

        if self.options.with_gui:
            self.requires("freeglut/3.2.1@{}".format(self._orbit_channel))
            self.requires("freetype/2.10.0@bincrafters/stable")
            self.requires("freetype-gl/8d9a97a@{}".format(self._orbit_channel))
            self.requires("glew/2.1.0@{}".format(self._orbit_channel))
            self.requires("imgui/1.69@bincrafters/stable")
            self.requires("libpng/1.6.37@bincrafters/stable")
            if not self.options.system_mesa:
                self.requires("libxi/1.7.10@bincrafters/stable")
            if not self.options.system_qt:
                self.requires("qt/5.14.1@bincrafters/stable")

    def configure(self):
        self.options["abseil"].cxx_standard = 17
        if self.options.with_gui:
            self.options["glew"].system_mesa = self.options.system_mesa
            self.options["freeglut"].system_mesa = self.options.system_mesa

            if not self.options.system_qt:
                self.options["qt"].shared = True
                self.options["qt"].with_sqlite3 = False
                self.options["qt"].with_mysql = False
                self.options["qt"].with_pq = False
                self.options["qt"].with_odbc = False
                self.options["qt"].with_sdl2 = False
                self.options["qt"].with_openal = False

                if self.settings.os == "Windows":
                    self.options["qt"].qttools = True
                    self.options["qt"].with_glib = False
                    self.options["qt"].with_harfbuzz = False

    def build(self):
        cmake = CMake(self)
        cmake.definitions["WITH_GUI"] = "ON" if self.options.with_gui else "OFF"
        cmake.configure()
        cmake.build()
        if not self.options.ggp:
            cmake.test()

    def imports(self):
        dest = os.getenv("CONAN_IMPORT_PATH", "bin")
        self.copy("*.dll", src="@bindirs", dst=dest)
        self.copy("*.so*", src="@libdirs", dst=dest)
        if self.options.with_gui:
            for path in self.deps_cpp_info["freetype-gl"].resdirs:
                self.copy("Vera.ttf", src=path, dst="{}/fonts/".format(dest))
                self.copy("Vera.ttf", src=path, dst="{}/fonts/".format("OrbitQt/"))
                self.copy("v3f-t2f-c4f.*", src=path, dst="{}/shaders/".format(dest))
                self.copy("v3f-t2f-c4f.*", src=path, dst="{}/shaders/".format("OrbitQt/"))


    def package(self):
        self.copy("*", src="bin/dri", dst="bin/dri", symlinks=True)
        self.copy("*", src="bin/fonts", dst="bin/fonts", symlinks=True)
        self.copy("*", src="bin/shaders", dst="bin/shaders", symlinks=True)
        self.copy("*.so*", src="bin/", dst="bin", symlinks=True)
        if self.settings.os == "Windows":
            self.copy("*.dll", src="bin/", dst="bin", symlinks=True)
        self.copy("Orbit", src="bin/", dst="bin")
        self.copy("Orbit.exe", src="bin/", dst="bin")
        self.copy("Orbit.pdb", src="bin/", dst="bin")
        self.copy("OrbitService*", src="bin/", dst="bin")

    def deploy(self):
        self.copy("*", src="bin", dst="bin")
