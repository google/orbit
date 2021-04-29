from conans import ConanFile, Meson, tools


class LlvmpipeConan(ConanFile):
    name = "llvmpipe"
    version = "21.0.3"
    license = "MIT"
    author = "Henning Becker"
    description = "LLVMPIPE is an OpenGL software rasterizer"
    settings = "os", "compiler", "build_type", "arch"
    no_copy_source = True
    generators = "pkg_config"
    exports_sources = "patches/*"

    requires = [("llvm-core/11.1.0@orbitdeps/stable", "private"),
                ("zlib/1.2.11", "private")]
    build_requires = "winflexbison/2.5.24", "pkgconf/1.7.3", "meson/0.57.2"

    def source(self):
        tools.get(**self.conan_data["sources"][self.version])

        for patch in self.conan_data.get('patches', {}).get(self.version, []):
            tools.patch(**patch)

    def build(self):
        definitions  = {}
        definitions["gallium-drivers"]="swrast"
        definitions["llvm"]="enabled"
        definitions["shared-llvm"]="disabled"
        definitions["llvm-rtti"]="enabled" if self.options["llvm-core"].rtti else "disabled"

        meson = Meson(self)
        meson.configure(source_folder="mesa-{}".format(self.version), defs=definitions, args=["--debug"])
        meson.build()
        meson.install()

    def package(self):
        self.copy("*license.rst", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["opengl32"]

