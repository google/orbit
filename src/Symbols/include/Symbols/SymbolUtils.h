// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOLS_SYMBOL_UTILS_H_
#define SYMBOLS_SYMBOL_UTILS_H_

#include <stdint.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_symbols {

// The file extensions for symbols files are .debug for elf files and .pdb for coff files. Only
// files with the following formats are considered: `module.sym_ext`, `module.mod_ext.sym_ext` and
// `module.mod_ext` (`mod_ext` is the module file extension, usually .elf, .so, .exe or .dll;
// `sym_ext` is either .debug or .pdb).
[[nodiscard]] std::vector<std::filesystem::path> GetStandardSymbolFilenamesForModule(
    const std::filesystem::path& module_path,
    const orbit_grpc_protos::ModuleInfo::ObjectFileType& object_file_type);

// Checks if the file at symbol_file_path can be read as symbol file (elf, coff, pdb) and compares
// the build id of the file with build_id. Returns void if build ids are the same, ErrorMessage
// otherwise.
[[nodiscard]] ErrorMessageOr<void> VerifySymbolFile(const std::filesystem::path& symbol_file_path,
                                                    std::string_view build_id);
// Checks if the file at symbol_file_path can be read as symbol file (elf, coff, pdb) and compares
// the size of the file with expected_file_size. Returns void if the sizes are the same,
// ErrorMessage otherwise.
[[nodiscard]] ErrorMessageOr<void> VerifySymbolFile(const std::filesystem::path& symbol_file_path,
                                                    uint64_t expected_file_size);
// Checks if the file at object_file_path can be read as an object file (elf, coff) and compares the
// build id and size of the file with build_id and expected_file_size, respectively. Returns void if
// the build ids and the sizes are the same, ErrorMessage otherwise.
[[nodiscard]] ErrorMessageOr<void> VerifyObjectFile(const std::filesystem::path& object_file_path,
                                                    std::string_view build_id,
                                                    uint64_t expected_file_size);

}  // namespace orbit_symbols

#endif  // SYMBOLS_SYMBOL_UTILS_H_