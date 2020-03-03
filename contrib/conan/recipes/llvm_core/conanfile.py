from conans import python_requires
import os

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMCore(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_core'
    llvm_component = 'llvm'
    llvm_module = 'Core'
    llvm_requires = ['llvm_headers', 'llvm_binary_format', 'llvm_remarks',
                     'llvm_support']
