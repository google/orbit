from conans import python_requires
import os

common = python_requires('llvm-common/0.0.2@orbitdeps/stable')

class LLVMDebugInfoPDB(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_debuginfo_pdb'
    llvm_component = 'llvm'
    llvm_module = 'DebugInfoPDB'
    llvm_requires = ['llvm_headers', 'llvm_support', 'llvm_debuginfo_msf',
                     'llvm_debuginfo_codeview']
