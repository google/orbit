from conans import python_requires

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMAsmPrinter(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_asm_printer'
    llvm_component = 'llvm'
    llvm_module = 'AsmPrinter'
    llvm_requires = ['llvm_headers', 'llvm_analysis', 'llvm_binary_format', 'llvm_codegen', 'llvm_core', 'llvm_debuginfo_codeview', 'llvm_debuginfo_msf', 'llvm_mc', 'llvm_mc_parser', 'llvm_support', 'llvm_target']
