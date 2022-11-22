// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_SYMBOLS_FILE_H_
#define OBJECT_UTILS_SYMBOLS_FILE_H_

#include <stdint.h>

#include <filesystem>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "GrpcProtos/symbol.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_object_utils {

struct ObjectFileInfo {
  // For ELF, this is the load bias of the executable segment. For PE/COFF, we use ImageBase here,
  // so that our address computations are consistent between what we do for ELF and for COFF.
  uint64_t load_bias = 0;
};

class SymbolsFile {
 public:
  SymbolsFile() = default;
  virtual ~SymbolsFile() = default;

  // For ELF files, the string returned by GetBuildId() is the standard build id that can be found
  // in the .note.gnu.build-id section, formatted as a human-readable string.
  // PE/COFF object files are uniquely identified by the PDB debug info consisting of a GUID and
  // age. The build id is formed from these to provide a string that uniquely identifies this object
  // file and the corresponding PDB debug info. The build id for PDB files is formed in the same
  // way.
  [[nodiscard]] virtual std::string GetBuildId() const = 0;
  [[nodiscard]] virtual ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadDebugSymbols() = 0;
  [[nodiscard]] virtual const std::filesystem::path& GetFilePath() const = 0;

  static constexpr uint64_t kUnknownSymbolSize = std::numeric_limits<uint64_t>::max();
  [[nodiscard]] static bool SymbolInfoLessByAddress(const orbit_grpc_protos::SymbolInfo& lhs,
                                                    const orbit_grpc_protos::SymbolInfo& rhs);
  static void DeduceDebugSymbolMissingSizesAsDistanceFromNextSymbol(
      std::vector<orbit_grpc_protos::SymbolInfo>* symbol_infos);
};

// Create a symbols file from the file at symbol_file_path. Additional info about the corresponding
// module can be passed in via object_file_info. This is necessary for PDB files, where information
// such as the load bias cannot be determined from the PDB file alone but is needed to compute the
// right addresses for symbols.
ErrorMessageOr<std::unique_ptr<SymbolsFile>> CreateSymbolsFile(
    const std::filesystem::path& symbol_file_path, const ObjectFileInfo& object_file_info);

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_SYMBOLS_FILE_H_