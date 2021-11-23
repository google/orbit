// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_COFF_FILE_H_
#define OBJECT_UTILS_COFF_FILE_H_

#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>

#include <array>
#include <filesystem>
#include <memory>
#include <optional>

#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/ObjectFile.h"
#include "OrbitBase/Result.h"

namespace orbit_object_utils {

struct PdbDebugInfo {
  std::filesystem::path pdb_file_path;

  std::array<uint8_t, 16> guid;
  uint32_t age;
};

class CoffFile : public ObjectFile {
 public:
  CoffFile() = default;
  virtual ~CoffFile() = default;

  [[nodiscard]] virtual ErrorMessageOr<PdbDebugInfo> GetDebugPdbInfo() const = 0;
};

[[nodiscard]] ErrorMessageOr<std::unique_ptr<CoffFile>> CreateCoffFile(
    const std::filesystem::path& file_path);
[[nodiscard]] ErrorMessageOr<std::unique_ptr<CoffFile>> CreateCoffFile(
    const std::filesystem::path& file_path,
    llvm::object::OwningBinary<llvm::object::ObjectFile>&& file);

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_COFF_FILE_H_
