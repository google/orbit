// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SymbolHelper.h"

#include <absl/strings/str_format.h>

#include <fstream>

#include "ElfFile.h"
#include "OrbitBase/Logging.h"
#include "Path.h"
#include "Pdb.h"

namespace {

std::vector<std::string> ReadSymbolsFile() {
  std::string file_name = Path::GetSymbolsFileName();
  if (!Path::FileExists(file_name)) {
    std::ofstream outfile(file_name);
    outfile << "//-------------------" << std::endl
            << "// Orbit Symbol Locations" << std::endl
            << "//-------------------" << std::endl
            << "// Orbit will scan the specified directories for symbol files."
            << std::endl
            << "// Enter one directory per line, like so:" << std::endl
#ifdef _WIN32
            << "// C:\\MyApp\\Release\\" << std::endl
            << "// D:\\MySymbolServer\\" << std::endl
#else
            << "// /home/git/project/build/" << std::endl
            << "// /home/symbol_server/" << std::endl
#endif
            << std::endl;

    outfile.close();
  }

  std::vector<std::string> directories;
  std::fstream infile(file_name);
  if (!infile.fail()) {
    std::string line;
    while (std::getline(infile, line)) {
      if (absl::StartsWith(line, "//") || line.empty()) continue;

      const std::string& dir = line;
      if (Path::DirExists(dir)) {
        directories.push_back(dir);
      } else {
        ERROR("Symbols directory \"%s\" doesn't exist", dir);
      }
    }
  }
  return directories;
}

bool LoadFromElfFile(std::shared_ptr<Module> module,
                     const std::unique_ptr<ElfFile>& elf_file) {
  auto load_bias = elf_file->GetLoadBias();
  if (!load_bias) {
    ERROR(
        "Unable to get load_bias %s (does the file have PT_LOAD program "
        "headers?)",
        elf_file->GetFilePath().c_str());
    return false;
  }

  std::shared_ptr<Pdb> pdb =
      std::make_shared<Pdb>(module->m_AddressStart, load_bias.value(),
                            elf_file->GetFilePath(), module->m_FullName);

  if (!elf_file->LoadFunctions(pdb.get())) {
    ERROR("Unable to load functions from %s", elf_file->GetFilePath().c_str());
    return false;
  }

  pdb->ProcessData();

  module->m_Pdb = pdb;
  module->m_PdbName = elf_file->GetFilePath();
  module->SetLoaded(true);
  return true;
}

bool FindAndLoadSymbols(std::shared_ptr<Module> module,
                        const std::vector<std::string>& search_directories) {
  FAIL_IF(module->m_DebugSignature.empty(), "build id of module is empty: %s",
          module->m_Name);
  std::string name_without_extension = Path::StripExtension(module->m_Name);

  std::vector<std::string> search_file_paths;
  for (const auto& directory : search_directories) {
    search_file_paths.emplace_back(
        Path::JoinPath({directory, name_without_extension + ".debug"}));
    search_file_paths.emplace_back(
        Path::JoinPath({directory, module->m_Name + ".debug"}));
    search_file_paths.emplace_back(Path::JoinPath({directory, module->m_Name}));
  }

  LOG("Trying to find symbols for module: %s", module->m_Name);
  for (const auto& symbols_file_path : search_file_paths) {
    if (!Path::FileExists(symbols_file_path)) continue;

    std::unique_ptr<ElfFile> symbols_file = ElfFile::Create(symbols_file_path);
    if (!symbols_file) continue;
    if (!symbols_file->HasSymtab()) continue;
    if (symbols_file->GetBuildId() != module->m_DebugSignature) continue;

    LOG("Loading symbols for module \"%s\" from \"%s\"", module->m_Name,
        symbols_file_path);
    return LoadFromElfFile(module, symbols_file);
  }

  return false;
}

bool CheckModuleHasBuildId(std::shared_ptr<Module> module) {
  if (module->m_DebugSignature.empty()) {
    ERROR(
        "Symbol loading from a separate file is not supported for module "
        "\"%s\", because it does not contain a build id. This likely means "
        "the build id has been turned off manually.",
        module->m_Name);
    return false;
  } else {
    return true;
  }
}

}  // namespace

SymbolHelper::SymbolHelper()
    : collector_symbol_directories_{"/home/cloudcast/",
                                    "/home/cloudcast/debug_symbols/",
                                    "/mnt/developer/",
                                    "/mnt/developer/debug_symbols/",
                                    "/srv/game/assets/",
                                    "/srv/game/assets/debug_symbols/"},
      symbols_file_directories_(ReadSymbolsFile()) {}

bool SymbolHelper::LoadSymbolsIncludedInBinary(
    std::shared_ptr<Module> module) const {
  std::unique_ptr<ElfFile> elf_file = ElfFile::Create(module->m_FullName);

  if (!elf_file) {
    ERROR("Unable to load ELF file: %s", module->m_FullName);
    return false;
  }

  if (!elf_file->HasSymtab()) return false;
  return LoadFromElfFile(module, elf_file);
}

outcome::result<ModuleSymbols, std::string> SymbolHelper::LoadSymbolsCollector(
    const std::string& module_path) const {
  std::unique_ptr<ElfFile> elf_file = ElfFile::Create(module_path);

  if (!elf_file) {
    return absl::StrFormat("Unable to load ELF file: \"%s\"", module_path);
  }

  const auto module = std::make_shared<Module>();
  module->m_FullName = module_path;
  module->m_DebugSignature = elf_file->GetBuildId();
  module->m_Name = Path::GetFileName(module_path);

  if (elf_file->HasSymtab()) {
    if (!LoadFromElfFile(module, elf_file)) {
      return absl::StrFormat("Error while loading symbols from file: \"%s\"",
                             module_path);
    }
  } else {
    std::vector<std::string> search_directories = collector_symbol_directories_;
    search_directories.emplace_back(Path::GetDirectory(module->m_FullName));

    if (!FindAndLoadSymbols(module, search_directories)) {
      return absl::StrFormat("No symbols found on remote for module \"%s\"",
                             module_path);
    }
  }

  ModuleSymbols module_symbols;
  module_symbols.set_load_bias(module->m_Pdb->GetLoadBias());
  module_symbols.set_symbols_file_path(module->m_Pdb->GetFileName());

  for (const auto& function : module->m_Pdb->GetFunctions()) {
    SymbolInfo* symbol_info = module_symbols.add_symbol_infos();
    symbol_info->set_name(function->Name());
    symbol_info->set_pretty_name(function->PrettyName());
    symbol_info->set_address(function->Address());
    symbol_info->set_size(function->Size());
    symbol_info->set_source_file(function->File());
    symbol_info->set_source_line(function->Line());
  }

  return module_symbols;
}

bool SymbolHelper::LoadSymbolsUsingSymbolsFile(
    std::shared_ptr<Module> module) const {
  if (!CheckModuleHasBuildId(module)) return false;

  return FindAndLoadSymbols(module, symbols_file_directories_);
}

void SymbolHelper::LoadSymbolsIntoModule(
    const std::shared_ptr<Module>& module,
    const ModuleSymbols& module_symbols) const {
  module->m_Pdb = std::make_shared<Pdb>(
      module->m_AddressStart, module_symbols.load_bias(),
      module_symbols.symbols_file_path(), module->m_FullName);

  for (const auto& symbol_info : module_symbols.symbol_infos()) {
    std::shared_ptr<Function> function = std::make_shared<Function>(
        symbol_info.name(), symbol_info.pretty_name(), symbol_info.address(),
        module_symbols.load_bias(), symbol_info.size(),
        symbol_info.source_file(), symbol_info.source_line());
    module->m_Pdb->AddFunction(std::move(function));
  }
  module->m_Pdb->ProcessData();
  module->m_PdbName = module_symbols.symbols_file_path();
  module->SetLoaded(true);
}
