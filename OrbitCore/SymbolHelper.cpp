// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SymbolHelper.h"

#include <absl/strings/match.h>
#include <absl/strings/str_format.h>

#include <fstream>

#include "ElfUtils/ElfFile.h"
#include "OrbitBase/Logging.h"
#include "Path.h"

namespace {

using ::ElfUtils::ElfFile;

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

ErrorMessageOr<ModuleSymbols> FindSymbols(
    const std::string& module_path, const std::string& build_id,
    const std::vector<std::string>& search_directories) {
  std::string filename = Path::GetFileName(module_path);
  std::string filename_without_extension = Path::StripExtension(filename);

  std::vector<std::string> search_file_paths;
  for (const auto& directory : search_directories) {
    search_file_paths.emplace_back(
        Path::JoinPath({directory, filename_without_extension + ".debug"}));
    search_file_paths.emplace_back(
        Path::JoinPath({directory, filename + ".debug"}));
    search_file_paths.emplace_back(Path::JoinPath({directory, filename}));
  }

  LOG("Trying to find symbols for module: \"%s\"", module_path);
  for (const auto& symbols_file_path : search_file_paths) {
    if (!Path::FileExists(symbols_file_path)) continue;

    ErrorMessageOr<std::unique_ptr<ElfFile>> symbols_file =
        ElfFile::Create(symbols_file_path);
    if (!symbols_file) continue;
    if (!symbols_file.value()->HasSymtab()) continue;
    if (symbols_file.value()->GetBuildId() != build_id) continue;

    LOG("Loading symbols for module \"%s\" from \"%s\"", module_path,
        symbols_file_path);
    return symbols_file.value()->LoadSymbols();
  }

  return ErrorMessage(
      absl::StrFormat("Could not find symbols for module \"%s\"", module_path));
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

ErrorMessageOr<ModuleSymbols> SymbolHelper::LoadSymbolsCollector(
    const std::string& module_path) const {
  ErrorMessageOr<std::unique_ptr<ElfFile>> elf_file_result =
      ElfFile::Create(module_path);

  if (!elf_file_result) {
    return ErrorMessage(absl::StrFormat("Unable to load ELF file: \"%s\": %s",
                                        module_path,
                                        elf_file_result.error().message()));
  }

  std::unique_ptr<ElfFile> elf_file = std::move(elf_file_result.value());

  if (elf_file->HasSymtab()) {
    return elf_file->LoadSymbols();
  }

  if (elf_file->GetBuildId().empty()) {
    return ErrorMessage(absl::StrFormat(
        "No symbols are contained in the module \"%s\". Symbols cannot be "
        "loaded from a separate symbols file, because module does not "
        "contain a build_id,",
        module_path));
  }

  std::vector<std::string> search_directories = collector_symbol_directories_;
  search_directories.emplace_back(Path::GetDirectory(module_path));

  return FindSymbols(module_path, elf_file->GetBuildId(), search_directories);
}

ErrorMessageOr<ModuleSymbols> SymbolHelper::LoadUsingSymbolsPathFile(
    const std::string& module_path, const std::string& build_id) const {
  return FindSymbols(module_path, build_id, symbols_file_directories_);
}
