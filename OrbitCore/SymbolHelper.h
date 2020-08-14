// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOL_HELPER_H_
#define SYMBOL_HELPER_H_

#include <string>
#include <vector>

#include "OrbitBase/Result.h"
#include "symbol.pb.h"

class SymbolHelper {
 public:
  SymbolHelper();
  SymbolHelper(std::vector<std::string> collector_symbol_directories,
               std::vector<std::string> symbols_file_directories)
      : collector_symbol_directories_(std::move(collector_symbol_directories)),
        symbols_file_directories_(std::move(symbols_file_directories)){};

  [[nodiscard]] ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadSymbolsCollector(
      const std::string& module_path) const;
  [[nodiscard]] ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadUsingSymbolsPathFile(
      const std::string& module_path, const std::string& build_id) const;
  [[nodiscard]] ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadSymbolsFromFile(
      const std::string& file_path, const std::string& build_id) const;

  [[nodiscard]] ErrorMessageOr<std::string> FindDebugSymbolsFile(const std::string& module_path,
                                                                 const std::string& build_id) const;

  [[nodiscard]] std::string GenerateCachedFileName(const std::string& file_path) const;

 private:
  const std::vector<std::string> collector_symbol_directories_;
  const std::vector<std::string> symbols_file_directories_;
};

#endif  // SYMBOL_HELPER_H_
