// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_SYMBOLS_FILE_H_
#define OBJECT_UTILS_SYMBOLS_FILE_H_

#include <filesystem>
#include <memory>
#include <string>

#include "OrbitBase/Result.h"
#include "symbol.pb.h"

namespace orbit_object_utils {

class SymbolsFile {
 public:
  SymbolsFile() = default;
  virtual ~SymbolsFile() = default;

  // For ELF files, the string returned by GetBuildId() is the standard build id that can be found
  // in the .note.gnu.build-id section, formatted as a human readable string.
  // PE/COFF object files are uniquely identfied by the PDB debug info consisting of a GUID and age.
  // The build id is formed from these to provide a string that uniquely identifies this object file
  // and the corresponding PDB debug info. The build id for PDB files is formed in the same way.
  [[nodiscard]] virtual std::string GetBuildId() const = 0;
  [[nodiscard]] virtual ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadDebugSymbols() = 0;
  [[nodiscard]] virtual const std::filesystem::path& GetFilePath() const = 0;
};

ErrorMessageOr<std::unique_ptr<SymbolsFile>> CreateSymbolsFile(
    const std::filesystem::path& file_path);

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_SYMBOLS_FILE_H_