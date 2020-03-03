from conans import python_requires

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMBinaryFormat(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_binary_format'
    llvm_component = 'llvm'
    llvm_module = 'BinaryFormat'
    llvm_requires = ['llvm_headers', 'llvm_support']
