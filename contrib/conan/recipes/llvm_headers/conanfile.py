# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from conans import python_requires

common = python_requires('llvm-common/0.0.0@orbitdeps/stable')

class LLVMHeaders(common.LLVMModulePackage):
    version = common.LLVMModulePackage.version
    name = 'llvm_headers'
    llvm_component = 'llvm'
    header_only = True
    include_dirs = ['']

    def package_info(self):
        super().package_info()

        if self.settings.os == "Linux":
            self.cpp_info.defines.append("LLVM_ON_UNIX")
