from conans import python_requires

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMSupport(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_support'
    llvm_component = 'llvm'
    llvm_module = 'Support'
    llvm_requires = ['llvm_headers', 'llvm_demangle']

    def requirements(self):
        self.requires('zlib/1.2.11@conan/stable')
        super().requirements()

    def package_info(self):
        super().package_info()
        if self.settings.os != "Windows":
            self.cpp_info.system_libs.append('pthread')
            self.cpp_info.system_libs.append('dl')
