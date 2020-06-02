from conans import python_requires

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMAnalysis(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_analysis'
    llvm_component = 'llvm'
    llvm_module = 'Analysis'
    llvm_requires = ['llvm_headers', 'llvm_binary_format', 'llvm_core', 'llvm_object', 'llvm_profile_data', 'llvm_support']
