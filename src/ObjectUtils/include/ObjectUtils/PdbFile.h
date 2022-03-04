// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_PDB_FILE_H_
#define OBJECT_UTILS_PDB_FILE_H_

#include <array>
#include <filesystem>

#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/SymbolsFile.h"
#include "OrbitBase/Result.h"

namespace orbit_object_utils {

class PdbFile : public SymbolsFile {
 public:
  PdbFile() = default;
  virtual ~PdbFile() = default;

  // GUID and Age are used to match PDB files to objects, similar to build id in the ELF case.
  [[nodiscard]] virtual std::array<uint8_t, 16> GetGuid() const = 0;
  [[nodiscard]] virtual uint32_t GetAge() const = 0;
};

ErrorMessageOr<std::unique_ptr<PdbFile>> CreatePdbFile(const std::filesystem::path& file_path,
                                                       const ObjectFileInfo& object_file_info);

#if WIN32
enum class PdbParserType { Llvm, Dia };
ErrorMessageOr<std::unique_ptr<PdbFile>> CreatePdbFile(const std::filesystem::path& file_path,
                                                       const ObjectFileInfo& object_file_info,
                                                       PdbParserType parser_type);
#endif

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_PDB_FILE_H_
