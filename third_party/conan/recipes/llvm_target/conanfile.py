from conans import python_requires

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMTarget(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_target'
    llvm_component = 'llvm'
    llvm_module = 'Target'
    llvm_requires = ['llvm_headers', 'llvm_analysis', 'llvm_core', 'llvm_mc', 'llvm_support']
