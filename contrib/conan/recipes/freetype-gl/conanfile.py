from conans import ConanFile, CMake, tools
import shutil, os


class FreetypeglConan(ConanFile):
    name = "freetype-gl"
    version = "8d9a97a"
    license = "BSD-2-Clause"
    description = "freetype-gl is a small library for displaying Unicode in OpenGL"
    topics = ("freetype", "opengl", "unicode", "fonts")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}
    generators = "cmake", "cmake_find_package_multi"
    requires = "glew/2.1.0@orbitdeps/stable", "freetype/2.10.0@bincrafters/stable", "zlib/1.2.11@conan/stable"

    def source(self):
        self.run("git clone https://github.com/rougier/freetype-gl.git")
        self.run("git checkout {}".format(self.version), cwd="freetype-gl/")
        # This small hack might be useful to guarantee proper /MT /MD linkage
        # in MSVC if the packaged project doesn't have variables to set it
        # properly
        tools.replace_in_file("freetype-gl/CMakeLists.txt",
                              "project(freetype-gl LANGUAGES C CXX)",
                              '''project(freetype-gl LANGUAGES C CXX)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()''')
        tools.replace_in_file("freetype-gl/CMakeLists.txt",
                              "find_package(Freetype REQUIRED)",
                              "find_package(Freetype CONFIG REQUIRED)")
        os.remove("freetype-gl/cmake/Modules/FindGLEW.cmake")

    def _get_cmake(self):
        cmake = CMake(self)
        cmake.definitions["freetype-gl_WITH_GLEW"] = True
        cmake.definitions["freetype-gl_USE_VAO"] = False
        cmake.definitions["freetype-gl_BUILD_DEMOS"] = False
        cmake.definitions["freetype-gl_BUILD_APIDOC"] = False
        cmake.definitions["freetype-gl_BUILD_HARFBUZZ"] = False
        cmake.definitions["freetype-gl_BUILD_MAKEFONT"] = False
        cmake.definitions["freetype-gl_BUILD_TESTS"] = False
        cmake.configure(source_folder="freetype-gl")
        return cmake

    def build(self):
        if "mesa" in self.deps_cpp_info.deps:
            mesa_path = self.deps_cpp_info["mesa"].rootpath
            shutil.copyfile(os.path.join(mesa_path, "lib", "pkgconfig", "gl.pc"), "gl.pc")
            tools.replace_prefix_in_pc_file("gl.pc", mesa_path)

        with tools.environment_append({"PKG_CONFIG_PATH": os.getcwd()}):
            cmake = self._get_cmake()
            cmake.build()

    def package(self):
        cmake = self._get_cmake()
        cmake.install()
        self.copy("*", src="freetype-gl/fonts/", dst="fonts/")
        self.copy("*", src="freetype-gl/shaders/", dst="shaders/")

    def package_info(self):
        if self.settings.os == "Windows":
            self.cpp_info.libs = ["freetype-gl.lib"]
        else:
            self.cpp_info.libs = ["libfreetype-gl.a"]
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.resdirs = ["fonts", "shaders"]

