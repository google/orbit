// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOLS_SYMBOL_UTILS_H_
#define SYMBOLS_SYMBOL_UTILS_H_

#include <filesystem>
#include <vector>

#include "GrpcProtos/module.pb.h"

namespace orbit_symbols {

// The file extensions for symbols files are .debug for elf files and .pdb for coff files. Only
// files with the following formats are considered: `module.sym_ext`, `module.mod_ext.sym_ext` and
// `module.mod_ext` (`mod_ext` is the module file extension, usually .elf, .so, .exe or .dll;
// `sym_ext` is either .debug or .pdb).
[[nodiscard]] std::vector<std::filesystem::path> GetStandardSymbolFilenamesForModule(
    const std::filesystem::path& module_path,
    const orbit_grpc_protos::ModuleInfo::ObjectFileType& object_file_type);

}  // namespace orbit_symbols

#endif  // SYMBOLS_SYMBOL_UTILS_H_