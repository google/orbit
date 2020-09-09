// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOL_HELPER_H_
#define SYMBOL_HELPER_H_

#include <filesystem>
#include <string>
#include <vector>

#include "OrbitBase/Result.h"
#include "symbol.pb.h"

namespace fs = std::filesystem;

class SymbolHelper {
 public:
  SymbolHelper();
  explicit SymbolHelper(std::vector<fs::path> symbols_file_directories, fs::path cache_directory)
      : symbols_file_directories_(std::move(symbols_file_directories)),
        cache_directory_(std::move(cache_directory)){};

  [[nodiscard]] ErrorMessageOr<fs::path> FindSymbolsWithSymbolsPathFile(
      const fs::path& module_path, const std::string& build_id) const;
  [[nodiscard]] ErrorMessageOr<fs::path> FindSymbolsInCache(const fs::path& module_path,
                                                            const std::string& build_id) const;
  [[nodiscard]] static ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadSymbolsFromFile(
      const fs::path& file_path);
  [[nodiscard]] static ErrorMessageOr<void> VerifySymbolsFile(const fs::path& symbols_path,
                                                              const std::string& build_id);

  [[nodiscard]] fs::path GenerateCachedFileName(const fs::path& file_path) const;

 private:
  const std::vector<fs::path> symbols_file_directories_;
  const fs::path cache_directory_;
};

#endif  // SYMBOL_HELPER_H_
