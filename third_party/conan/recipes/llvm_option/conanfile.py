from conans import python_requires
import os

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMOption(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_option'
    llvm_component = 'llvm'
    llvm_module = 'Option'
    llvm_requires = ['llvm_headers', 'llvm_support']
