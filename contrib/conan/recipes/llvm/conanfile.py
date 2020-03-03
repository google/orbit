from conans import python_requires
llvm_common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMConan(llvm_common.LLVMComponentPackage):
    name = "llvm"
    version = llvm_common.LLVMComponentPackage.version

    custom_cmake_options = {
        'LLVM_BUILD_TOOLS': True
    }
    package_info_exclude_libs = [
        'BugpointPasses', 'LLVMHello', 'LTO'
    ]

    def requirements(self):
        self.requires('gtest/1.8.1@bincrafters/stable', private=True)
        super().requirements()
