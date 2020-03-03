from conans import ConanFile, CMake, tools
import os


class freeglutConan(ConanFile):
    name = "freeglut"
    version = "3.2.1"
    description = "Open-source alternative to the OpenGL Utility Toolkit (GLUT) library"
    topics = ("conan", "freeglut", "opengl", "gl", "glut", "utility", "toolkit", "graphics")
    url = "https://github.com/bincrafters/conan-freeglut"
    homepage = "https://github.com/dcnieho/FreeGLUT"
    license = "X11"
    exports_sources = ["CMakeLists.txt", "*.patch"]
    generators = "cmake"
    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "gles": [True, False],
        "print_errors_at_runtime": [True, False],
        "print_warnings_at_runtime": [True, False],
        "replace_glut": [True, False],
        "install_pdb": [True, False],
        "system_mesa": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "gles": False,
        "print_errors_at_runtime": True,
        "print_warnings_at_runtime": True,
        "replace_glut": True,
        "install_pdb": False,
        "system_mesa": True
    }

    @property
    def _source_subfolder(self):
        return "source_subfolder"

    @property
    def _build_subfolder(self):
        return "build_subfolder"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC
            self.options.replace_glut = False
            self.options.system_mesa = True
        if self.settings.compiler != "Visual Studio":
            self.options.install_pdb = False

    def configure(self):
        del self.settings.compiler.libcxx
        del self.settings.compiler.cppstd

    def requirements(self):
        if self.settings.os == 'Linux' and not self.options.system_mesa:
            self.requires('mesa/19.3.1@bincrafters/stable')
            self.requires('mesa-glu/9.0.1@bincrafters/stable')
            self.requires('libxi/1.7.10@bincrafters/stable')


    def source(self):
        archive_url = "{}/archive/FG_{}.tar.gz".format(self.homepage, self.version.replace(".", "_"))
        tools.get(archive_url, sha256="23b93b77ac01bc70af0c56fa3316f25ea8f136603f93df319ca74867e15134d3")
        extracted_dir = "FreeGLUT-FG_" + self.version.replace(".", "_")
        os.rename(extracted_dir, self._source_subfolder)

        # on macOS GLX can't be found https://github.com/dcnieho/FreeGLUT/issues/27
        tools.patch(base_path=self._source_subfolder, patch_file=os.path.join("patch", "0002-macOS-Fix-GLX-not-found.patch"))

    def system_requirements(self):
        if self.settings.os == "Macos" and tools.os_info.is_macos:
            # INFO (uilian): SystemPackageTools doesn't offer cask
            self.run("brew cask install xquartz")

    def _configure_cmake(self):
        # See https://github.com/dcnieho/FreeGLUT/blob/44cf4b5b85cf6037349c1c8740b2531d7278207d/README.cmake
        cmake = CMake(self, set_cmake_flags=True)

        cmake.definitions["FREEGLUT_BUILD_DEMOS"] = "OFF"
        cmake.definitions["FREEGLUT_BUILD_STATIC_LIBS"] = "OFF" if self.options.shared else "ON"
        cmake.definitions["FREEGLUT_BUILD_SHARED_LIBS"] = "ON" if self.options.shared else "OFF"
        cmake.definitions["FREEGLUT_GLES"] = "ON" if self.options.gles else "OFF"
        cmake.definitions["FREEGLUT_PRINT_ERRORS"] = "ON" if self.options.print_errors_at_runtime else "OFF"
        cmake.definitions["FREEGLUT_PRINT_WARNINGS"] = "ON" if self.options.print_warnings_at_runtime else "OFF"
        cmake.definitions["FREEGLUT_INSTALL_PDB"] = "ON" if self.options.install_pdb else "OFF"
        cmake.definitions["INSTALL_PDB"] = False
        # cmake.definitions["FREEGLUT_WAYLAND"] = "ON" if self.options.wayland else "OFF" # nightly version only as of now

        cmake.configure(build_folder=self._build_subfolder)
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        self.copy(pattern="COPYING", dst="licenses", src=self._source_subfolder)
        cmake = self._configure_cmake()
        cmake.install()
        tools.rmdir(os.path.join(self.package_folder, "lib", "pkgconfig"))
        tools.rmdir(os.path.join(self.package_folder, "lib", "cmake"))

    def package_info(self):
        self.cpp_info.libdirs = ["lib", "lib64"]

        if self.options.replace_glut:
            self.cpp_info.libs = ["glut"]
        else:
            self.cpp_info.libs = tools.collect_libs(self)

        if self.settings.os == "Windows":
            if not self.options.shared:
                self.cpp_info.defines.append("FREEGLUT_STATIC=1")
            self.cpp_info.defines.append("FREEGLUT_LIB_PRAGMAS=0")
            self.cpp_info.system_libs.append("glu32")
            self.cpp_info.system_libs.append("opengl32")
            self.cpp_info.system_libs.append("gdi32")
            self.cpp_info.system_libs.append("winmm")
            self.cpp_info.system_libs.append("user32")

        if self.settings.os == "Macos":
            self.cpp_info.system_libs.extend(['GL', 'GLU'])
            self.cpp_info.libdirs.extend(['/System/Library/Frameworks/ImageIO.framework/Resources', '/opt/X11/lib', '/usr/X11/lib', '/usr/X11R6/lib'])

        if self.settings.os == "Linux":
            self.cpp_info.system_libs.append("pthread")
            self.cpp_info.system_libs.append("m")
            self.cpp_info.system_libs.append("dl")
            self.cpp_info.system_libs.append("rt")

        self.output.info(self.cpp_info.libs)
