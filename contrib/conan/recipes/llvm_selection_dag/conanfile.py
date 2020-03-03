from conans import python_requires
import os

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMSelectionDag(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_selection_dag'
    llvm_component = 'llvm'
    llvm_module = 'SelectionDAG'
    llvm_requires = ['llvm_headers', 'llvm_analysis', 'llvm_codegen', 'llvm_core', 'llvm_mc', 'llvm_support', 'llvm_target', 'llvm_transform_utils']
