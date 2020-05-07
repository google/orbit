from conans import ConanFile, CMake, MSBuild, tools
import os
import shutil


class BreakpadConan(ConanFile):
    description = "Breakpad tools for symbol uploading"
    name = 'breakpad'
    version = "216cea7b"
    license = 'https://chromium.googlesource.com/breakpad/breakpad/+/master/LICENSE'
    url = 'https://gitlab.com/ArsenStudio/ArsenEngine/dependencies/conan-breakpad'
    generators = 'cmake'
    branch = 'master'
    exports = ["CMakeLists.txt", "fix-unique_ptr.patch"]
    build_requires = "libdisasm/0.23@orbitdeps/stable"
    
    settings = 'os_build', 'arch_build', 'compiler', 'arch'
    options = {
        "fPIC": [True, False]
    }
    default_options = {
        "fPIC": True
    }
    
    _source_dir = "breakpad"

    def source(self):
        self.run( 'git clone https://chromium.googlesource.com/breakpad/breakpad')
        if self.settings.os_build == 'Linux':
            self.run(
                'git clone https://chromium.googlesource.com/linux-syscall-support breakpad/src/third_party/lss')
        self.run('git checkout {}'.format(self.version), cwd='breakpad/')
        shutil.move('CMakeLists.txt', self._source_dir)
        tools.patch(patch_file="fix-unique_ptr.patch",
                    base_path=self._source_dir)

    def build(self):
        cmake = CMake(self)
        
        if self.settings.os_build == "Windows":
            vcvars = tools.vcvars_dict(self.settings)
            cmake.definitions["VSINSTALLDIR"] = vcvars["VSINSTALLDIR"].rstrip('\\')

        cmake.configure(source_folder=self._source_dir)
        cmake.build()
        
    def copy_sym_upload_tools(self):
        if self.settings.os_build == "Windows":
            self.copy("dump_syms.exe", src=os.path.join(self.build_folder, "bin"), dst="bin")
            self.copy("symupload.exe", src=os.path.join(self.build_folder, "bin"), dst="bin")
        else:
            self.copy("dump_syms", src=os.path.join(self.build_folder, "bin"), dst="bin")
            self.copy("symupload", src=os.path.join(self.build_folder, "bin"), dst="bin")

    def package(self):
        self.copy(pattern="LICENSE", dst="licenses", src=self._source_dir,
                                     ignore_case=True, keep_path=False)                             
        self.copy_sym_upload_tools()

    def package_info(self):
        self.env_info.PATH.append(os.path.join(self.package_folder, "bin"))
        
    def package_id(self):
        del self.info.settings.compiler
        del self.info.settings.arch
        self.info.include_build_settings()