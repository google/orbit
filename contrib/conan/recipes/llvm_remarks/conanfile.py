from conans import python_requires
import os

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMObject(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_remarks'
    llvm_component = 'llvm'
    llvm_module = 'Remarks'
    llvm_requires = ['llvm_headers', 'llvm_bitstream_reader', 'llvm_support']
