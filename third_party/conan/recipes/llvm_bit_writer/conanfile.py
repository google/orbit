from conans import python_requires
import os

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMBitWriter(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_bit_writer'
    llvm_component = 'llvm'
    llvm_module = 'BitWriter'
    llvm_requires = ['llvm_headers', 'llvm_analysis', 'llvm_core', 'llvm_mc', 'llvm_object', 'llvm_support']
