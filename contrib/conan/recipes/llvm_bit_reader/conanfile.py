from conans import python_requires
import os

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMBitReader(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_bit_reader'
    llvm_component = 'llvm'
    llvm_module = 'BitReader'
    llvm_requires = ['llvm_headers', 'llvm_bitstream_reader', 'llvm_core', 'llvm_support']
