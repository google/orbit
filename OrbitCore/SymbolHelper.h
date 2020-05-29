// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOL_HELPER_H_
#define SYMBOL_HELPER_H_
#include <string>
#include <vector>

#include "OrbitModule.h"
#include "Pdb.h"

class SymbolHelper {
 public:
  SymbolHelper();
  SymbolHelper(std::vector<std::string> collector_symbol_directories,
               std::vector<std::string> symbols_file_directories)
      : collector_symbol_directories_(std::move(collector_symbol_directories)),
        symbols_file_directories_(std::move(symbols_file_directories)){};

  bool LoadSymbolsIncludedInBinary(std::shared_ptr<Module> module) const;
  bool LoadSymbolsCollector(std::shared_ptr<Module> module) const;
  bool LoadSymbolsUsingSymbolsFile(std::shared_ptr<Module> module) const;
  void LoadSymbolsFromDebugInfo(std::shared_ptr<Module> module,
                                const ModuleDebugInfo& module_info) const;
  void FillDebugInfoFromModule(std::shared_ptr<Module> module,
                               ModuleDebugInfo* module_info) const;

  std::vector<std::string> GetSymbolsFileDirectories() const {
    return symbols_file_directories_;
  }

 private:
  const std::vector<std::string> collector_symbol_directories_;
  const std::vector<std::string> symbols_file_directories_;
};

#endif  // SYMBOL_HELPER_H_