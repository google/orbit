from conans import python_requires

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMX86Codegen(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_x86_codegen'
    llvm_component = 'llvm'
    llvm_module = 'X86CodeGen'
    llvm_requires = ['llvm_headers', 'llvm_analysis', 'llvm_asm_printer', 'llvm_codegen', 'llvm_core', 'llvm_global_isel', 'llvm_mc', 'llvm_selection_dag', 'llvm_support', 'llvm_x86_asm_printer', 'llvm_x86_desc', 'llvm_x86_info', 'llvm_support']
