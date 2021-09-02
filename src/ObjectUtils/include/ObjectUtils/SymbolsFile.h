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

  [[nodiscard]] virtual std::string GetBuildId() const = 0;
  [[nodiscard]] virtual ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadDebugSymbols() = 0;
  [[nodiscard]] virtual const std::filesystem::path& GetFilePath() const = 0;
};

ErrorMessageOr<std::unique_ptr<SymbolsFile>> CreateSymbolsFile(
    const std::filesystem::path& file_path);

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_SYMBOLS_FILE_H_