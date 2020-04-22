from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
import os
import shutil
from io import StringIO


class OrbitConan(ConanFile):
    name = "OrbitProfiler"
    license = "BSD-2-Clause"
    url = "https://github.com/pierricgimmig/orbitprofiler.git"
    description = "C/C++ Performance Profiler"
    settings = "os", "compiler", "build_type", "arch"
    generators = ["cmake_find_package_multi", "cmake"]
    options = {"system_mesa": [True, False],
               "system_qt": [True, False], "with_gui": [True, False],
               "debian_packaging": [True, False],
               "fPIC": [True, False]}
    default_options = {"system_mesa": True,
                       "system_qt": True, "with_gui": True,
                       "debian_packaging": False,
                       "fPIC": True}
    _orbit_channel = "orbitdeps/stable"
    exports_sources = "CMakeLists.txt", "Orbit*", "bin/*", "cmake/*", "external/*", "LICENSE"

    def _version(self):
        if not self.version:
            buf = StringIO()
            self.run("git describe --always --tags", output=buf)
            self.version = buf.getvalue().strip()[1:]

        return self.version


    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC


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
        self.requires("llvm_object/9.0.1@orbitdeps/stable")
        self.requires("openssl/1.1.1d@{}".format(self._orbit_channel))
        self.requires("Outcome/3dae433e@orbitdeps/stable")
        if self.settings.os != "Windows":
            self.requires(
                "libunwindstack/80a734f14@{}".format(self._orbit_channel))
        self.requires("zlib/1.2.11@conan/stable")

        self.requires("crashpad/20191009@{}".format(self._orbit_channel))

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
        if self.options.debian_packaging and (self.settings.get_safe("os.platform") != "GGP" or tools.detected_os() != "Linux"):
            raise ConanInvalidConfiguration(
                "Debian packaging is only supported for GGP builds!")

        if self.settings.os != "Windows" and not self.options.fPIC:
            raise ConanInvalidConfiguration("We only support compiling with fPIC enabled!")


        if self.options.with_gui and self.settings.arch == "x86":
            raise ConanInvalidConfiguration(
                "We don't actively support building the UI for 32bit platforms. Please remove this check in conanfile.py if you still want to do so!")

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
        if not tools.cross_building(self.settings, skip_x64_x86=True) and self.settings.get_safe("os.platform") != "GGP":
            cmake.test()

    def imports(self):
        dest = os.getenv("CONAN_IMPORT_PATH", "bin")
        self.copy("*.dll", src="@bindirs", dst=dest)
        self.copy("*.so*", src="@libdirs", dst=dest)
        self.copy("crashpad_handler*", src="@bindirs", dst=dest, root_package="crashpad")
        if self.options.with_gui:
            for path in self.deps_cpp_info["freetype-gl"].resdirs:
                self.copy("Vera.ttf", src=path, dst="{}/fonts/".format(dest))
                self.copy("Vera.ttf", src=path,
                          dst="{}/fonts/".format("OrbitQt/"))
                self.copy("v3f-t2f-c4f.*", src=path,
                          dst="{}/shaders/".format(dest))
                self.copy("v3f-t2f-c4f.*", src=path,
                          dst="{}/shaders/".format("OrbitQt/"))

    def package(self):
        if self.options.debian_packaging:
            shutil.rmtree(self.package_folder)
            self.copy("*.so*", src="bin/", dst="{}-{}/usr/lib/x86_64-linux-gnu/".format(
                self.name, self._version()), symlinks=True)
            self.copy("OrbitService", src="bin/",
                      dst="{}-{}/usr/bin/".format(self.name, self._version()))
            basedir = "{}/{}-{}".format(self.package_folder,
                                        self.name, self._version())
            os.makedirs("{}/DEBIAN".format(basedir), exist_ok=True)
            tools.save("{}/DEBIAN/control".format(basedir), """Package: orbitprofiler
Version: {}
Section: development
Priority: optional
Architecture: amd64
Maintainer: Google, Inc <orbitprofiler-eng@google.com>
Description: Orbit is a C/C++ profiler for Windows, Linux and the Stadia Platform.
Homepage: https://github.com/google/orbit
Installed-Size: `du -ks usr/ | cut -f 1`
""".format(self._version()))

            tools.save("{}/DEBIAN/postinst".format(basedir), """
#!/bin/bash
# Setting the setuid-bit for OrbitService
chmod -v 4775 /usr/bin/OrbitService
""")

            self.run("chmod +x {}/DEBIAN/postinst".format(basedir))
            self.run("chmod g-s {}/DEBIAN".format(basedir))
            self.run("chmod g-s {}/".format(basedir))
            self.run("dpkg-deb -b --root-owner-group {}".format(basedir))
            self.run("dpkg --contents {}.deb".format(basedir))
            shutil.rmtree(basedir)

        self.copy("*", src="bin/dri", dst="bin/dri", symlinks=True)
        self.copy("*", src="bin/fonts", dst="bin/fonts", symlinks=True)
        self.copy("*", src="bin/shaders", dst="bin/shaders", symlinks=True)
        self.copy("*.so*", src="bin/", dst="bin", symlinks=True)
        self.copy("*.dll", src="bin/", dst="bin", symlinks=True)
        self.copy("Orbit", src="bin/", dst="bin")
        self.copy("Orbit.exe", src="bin/", dst="bin")
        self.copy("Orbit.pdb", src="bin/", dst="bin")
        self.copy("Orbit.debug", src="bin/", dst="bin")
        self.copy("OrbitService", src="bin/", dst="bin")
        self.copy("OrbitService.exe", src="bin/", dst="bin")
        self.copy("OrbitService.pdb", src="bin/", dst="bin")
        self.copy("OrbitService.debug", src="bin/", dst="bin")

    def deploy(self):
        self.copy("*", src="bin", dst="bin")
