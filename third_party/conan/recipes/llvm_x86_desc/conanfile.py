from conans import python_requires

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMX86Desc(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_x86_desc'
    llvm_component = 'llvm'
    llvm_module = 'X86Desc'
    llvm_requires = ['llvm_headers', 'llvm_mc', 'llvm_mc_disassembler', 'llvm_support', 'llvm_x86_asm_printer', 'llvm_x86_info']
