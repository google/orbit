from conans import python_requires
import os

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMScalarOpts(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_scalar_opts'
    llvm_component = 'llvm'
    llvm_module = 'ScalarOpts'
    llvm_requires = ['llvm_headers', 'llvm_analysis', 'llvm_core', 'llvm_instcombine', 'llvm_support', 'llvm_transform_utils']
