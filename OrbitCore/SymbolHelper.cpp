// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SymbolHelper.h"

#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>

#include <fstream>

#include "ElfUtils/ElfFile.h"
#include "OrbitBase/Logging.h"
#include "Path.h"

using orbit_grpc_protos::ModuleSymbols;

namespace {

using ::ElfUtils::ElfFile;

std::vector<std::string> ReadSymbolsFile() {
  std::string file_name = Path::GetSymbolsFileName();
  if (!std::filesystem::exists(file_name)) {
    std::ofstream outfile(file_name);
    outfile << "//-------------------" << std::endl
            << "// Orbit Symbol Locations" << std::endl
            << "//-------------------" << std::endl
            << "// Orbit will scan the specified directories for symbol files." << std::endl
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

ErrorMessageOr<std::string> FindSymbolsFile(const std::string& module_path,
                                            const std::vector<std::string>& search_paths,
                                            const std::string& build_id) {
  std::string filename = Path::GetFileName(module_path);
  std::string filename_without_extension = Path::StripExtension(filename);

  std::vector<std::string> search_file_paths;
  search_file_paths.push_back(module_path);

  for (const auto& directory : search_paths) {
    search_file_paths.emplace_back(
        Path::JoinPath({directory, filename_without_extension + ".debug"}));
    search_file_paths.emplace_back(Path::JoinPath({directory, filename + ".debug"}));
    search_file_paths.emplace_back(Path::JoinPath({directory, filename}));
  }

  LOG("Trying to find symbols for module: \"%s\"", module_path);
  for (const auto& symbols_file_path : search_file_paths) {
    if (!std::filesystem::exists(symbols_file_path)) continue;

    ErrorMessageOr<std::unique_ptr<ElfFile>> symbols_file = ElfFile::Create(symbols_file_path);
    if (!symbols_file) continue;
    if (!symbols_file.value()->HasSymtab()) continue;

    // don't check build_id of module_path itself has symbols
    // and requested build_id is empty.
    if (build_id.empty() && symbols_file_path == module_path) {
      return symbols_file_path;
    }

    // Otherwise check that build_id is not empty and correct.
    if (symbols_file.value()->GetBuildId().empty()) continue;
    if (symbols_file.value()->GetBuildId() != build_id) continue;

    LOG("Found debug info for module \"%s\" -> \"%s\"", module_path, symbols_file_path);
    return symbols_file_path;
  }

  return ErrorMessage(
      absl::StrFormat("Could not find a file with debug symbols for module \"%s\"", module_path));
}

ErrorMessageOr<ModuleSymbols> FindSymbols(const std::string& module_path,
                                          const std::string& build_id,
                                          const std::vector<std::string>& search_paths) {
  ErrorMessageOr<std::string> debug_info_file_path =
      FindSymbolsFile(module_path, search_paths, build_id);

  if (!debug_info_file_path) {
    return debug_info_file_path.error();
  }

  ErrorMessageOr<std::unique_ptr<ElfFile>> elf_file_result =
      ElfFile::Create(debug_info_file_path.value());

  if (!elf_file_result) {
    return ErrorMessage(absl::StrFormat("Failed to load debug symbols for \"%s\" from \"%s\": %s",
                                        module_path, debug_info_file_path.value(),
                                        elf_file_result.error().message()));
  }

  return elf_file_result.value()->LoadSymbols();
}

}  // namespace

SymbolHelper::SymbolHelper()
    : collector_symbol_directories_{"/home/cloudcast/",  "/home/cloudcast/debug_symbols/",
                                    "/mnt/developer/",   "/mnt/developer/debug_symbols/",
                                    "/srv/game/assets/", "/srv/game/assets/debug_symbols/"},
      symbols_file_directories_(ReadSymbolsFile()) {}

ErrorMessageOr<ModuleSymbols> SymbolHelper::LoadSymbolsCollector(
    const std::string& module_path) const {
  ErrorMessageOr<std::unique_ptr<ElfFile>> elf_file_result = ElfFile::Create(module_path);

  if (!elf_file_result) {
    return ErrorMessage(absl::StrFormat("Unable to load ELF file: \"%s\": %s", module_path,
                                        elf_file_result.error().message()));
  }

  std::unique_ptr<ElfFile> elf_file = std::move(elf_file_result.value());

  if (elf_file->HasSymtab()) {
    return elf_file->LoadSymbols();
  }

  if (elf_file->GetBuildId().empty()) {
    return ErrorMessage(
        absl::StrFormat("No symbols are contained in the module \"%s\". Symbols cannot be "
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

ErrorMessageOr<std::string> SymbolHelper::FindDebugSymbolsFile(const std::string& module_path,
                                                               const std::string& build_id) const {
  return FindSymbolsFile(module_path, collector_symbol_directories_, build_id);
}

ErrorMessageOr<ModuleSymbols> SymbolHelper::LoadSymbolsFromFile(const std::string& file_path,
                                                                const std::string& build_id) const {
  ErrorMessageOr<std::unique_ptr<ElfFile>> elf_file_result = ElfFile::Create(file_path);

  if (!elf_file_result) {
    return ErrorMessage(absl::StrFormat("Failed to load debug symbols from \"%s\": %s", file_path,
                                        elf_file_result.error().message()));
  }

  const std::string& target_build_id = elf_file_result.value()->GetBuildId();
  if (target_build_id != build_id) {
    return ErrorMessage(
        absl::StrFormat("Failed to load debug symbols from \"%s\": invalid "
                        "build id \"%s\", expected: \"%s\"",
                        file_path, target_build_id, build_id));
  }

  return elf_file_result.value()->LoadSymbols();
}

std::string SymbolHelper::GenerateCachedFileName(const std::string& file_path) const {
  auto file_name = absl::StrReplaceAll(file_path, {{"/", "_"}});
  return Path::JoinPath({Path::GetCachePath(), file_name});
}
