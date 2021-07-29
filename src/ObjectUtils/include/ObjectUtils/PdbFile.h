// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_PDB_FILE_H_
#define OBJECT_UTILS_PDB_FILE_H_

#include <array>
#include <filesystem>

#include "OrbitBase/Result.h"
#include "symbol.pb.h"

namespace orbit_object_utils {

class PdbFile {
 public:
  PdbFile() = default;
  virtual ~PdbFile() = default;

  [[nodiscard]] virtual ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadDebugSymbols() = 0;
  [[nodiscard]] virtual const std::filesystem::path& GetFilePath() const = 0;

  // GUID and Age are used to match PDB files to objects, similar to build id in the ELF case.
  [[nodiscard]] virtual std::array<uint8_t, 16> GetGuid() const = 0;
  [[nodiscard]] virtual uint32_t GetAge() const = 0;
};

ErrorMessageOr<std::unique_ptr<PdbFile>> CreatePdbFile(const std::filesystem::path& file_path);

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_PDB_FILE_H_