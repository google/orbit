from conans import python_requires
import os

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMMCParser(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_mc_parser'
    llvm_component = 'llvm'
    llvm_module = 'MCParser'
    llvm_requires = ['llvm_headers', 'llvm_mc', 'llvm_support']
