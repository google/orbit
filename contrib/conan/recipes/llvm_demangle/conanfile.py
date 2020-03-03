from conans import python_requires
import os

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMDemangle(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_demangle'
    llvm_component = 'llvm'
    llvm_module = 'Demangle'
    llvm_requires = ['llvm_headers']
