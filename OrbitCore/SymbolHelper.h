// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOL_HELPER_H_
#define SYMBOL_HELPER_H_
#include <outcome.hpp>
#include <string>
#include <vector>

#include "OrbitModule.h"
#include "Pdb.h"
#include "symbol.pb.h"

class SymbolHelper {
 public:
  SymbolHelper();
  SymbolHelper(std::vector<std::string> collector_symbol_directories,
               std::vector<std::string> symbols_file_directories)
      : collector_symbol_directories_(std::move(collector_symbol_directories)),
        symbols_file_directories_(std::move(symbols_file_directories)){};

  bool LoadSymbolsIncludedInBinary(std::shared_ptr<Module> module) const;
  outcome::result<ModuleSymbols, std::string> LoadSymbolsCollector(
      const std::string& module_path) const;
  bool LoadSymbolsUsingSymbolsFile(std::shared_ptr<Module> module) const;
  void LoadSymbolsIntoModule(const std::shared_ptr<Module>& module,
                             const ModuleSymbols& module_symbols) const;

  std::vector<std::string> GetSymbolsFileDirectories() const {
    return symbols_file_directories_;
  }

 private:
  const std::vector<std::string> collector_symbol_directories_;
  const std::vector<std::string> symbols_file_directories_;
};

#endif  // SYMBOL_HELPER_H_