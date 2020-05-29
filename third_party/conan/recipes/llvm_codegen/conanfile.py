from conans import python_requires

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMCodegen(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_codegen'
    llvm_component = 'llvm'
    llvm_module = 'CodeGen'
    llvm_requires = ['llvm_headers', 'llvm_analysis', 'llvm_bit_reader', 'llvm_bit_writer', 'llvm_core', 'llvm_mc', 'llvm_profile_data', 'llvm_scalar_opts', 'llvm_support', 'llvm_target', 'llvm_transform_utils']
