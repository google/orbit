from conans import python_requires
import os

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMObject(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_object'
    llvm_component = 'llvm'
    llvm_module = 'Object'
    llvm_requires = ['llvm_headers', 'llvm_binary_format', 'llvm_bit_reader',
                     'llvm_core', 'llvm_mc', 'llvm_mc_parser', 'llvm_support',
                     'llvm_textapi']
