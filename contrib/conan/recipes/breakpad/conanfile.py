from conans import ConanFile, CMake, MSBuild, tools
import os
import shutil


class BreakpadConan(ConanFile):
    description = "Breakpad is a set of client and server components which implement a crash-reporting system."
    name = 'breakpad'
    version = "216cea7b"
    license = 'https://chromium.googlesource.com/breakpad/breakpad/+/master/LICENSE'
    url = 'https://gitlab.com/ArsenStudio/ArsenEngine/dependencies/conan-breakpad'
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake'
    branch = 'master'
    exports = ["CMakeLists.txt", "fix-unique_ptr.patch"]
    build_requires = "libdisasm/0.23@orbitdeps/stable"

    def get_env(self):
        return CMake(self)

    def source(self):
        self.run( 'git clone https://chromium.googlesource.com/breakpad/breakpad')
        if self.settings.os == 'Linux':
            self.run(
                'git clone https://chromium.googlesource.com/linux-syscall-support breakpad/src/third_party/lss')
        self.run('git checkout {}'.format(self.version), cwd='breakpad/')
        shutil.move('CMakeLists.txt', 'breakpad')
        tools.patch(patch_file="fix-unique_ptr.patch",
                    base_path="breakpad/")

    def build(self):
        env = self.get_env()
        env.definitions["INSTALL_HEADERS"] = True

        if self.settings.os == "Windows":
            vcvars = tools.vcvars_dict(self.settings)
            env.definitions["VSINSTALLDIR"] = vcvars["VSINSTALLDIR"].rstrip('\\')

        env.configure(source_folder="breakpad")
        env.build()

    def package(self):
        env = self.get_env()
        env.install()

    def package_info(self):
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.libs = ['breakpad']
        if self.settings.os == "Windows":
            self.cpp_info.libs.append('breakpad_client')

        self.cpp_info.includedirs = ['include', 'include/google_breakpad']
