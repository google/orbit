// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SymbolHelper.h"

#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>

#include <filesystem>
#include <fstream>

#include "ElfUtils/ElfFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/Tracing.h"
#include "Path.h"

using orbit_grpc_protos::ModuleSymbols;

namespace fs = std::filesystem;
using ::ElfUtils::ElfFile;

namespace {

std::vector<fs::path> ReadSymbolsFile() {
  std::string file_name = Path::GetSymbolsFileName();
  if (!fs::exists(file_name)) {
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

  std::vector<fs::path> directories;
  std::fstream infile(file_name);
  if (!infile.fail()) {
    std::string line;
    while (std::getline(infile, line)) {
      if (absl::StartsWith(line, "//") || line.empty()) continue;

      const fs::path& dir = line;
      if (std::filesystem::is_directory(dir)) {
        directories.push_back(dir);
      } else {
        ERROR("Symbols directory \"%s\" doesn't exist", dir.string());
      }
    }
  }
  return directories;
}

}  // namespace

ErrorMessageOr<void> SymbolHelper::VerifySymbolsFile(const fs::path& symbols_path,
                                                     const std::string& build_id) {
  OUTCOME_TRY(symbols_file, ElfFile::Create(symbols_path.string()));

  if (!symbols_file->HasSymtab()) {
    return ErrorMessage(
        absl::StrFormat("Elf file \"%s\" does not contain symbols.", symbols_path.string()));
  }
  if (symbols_file->GetBuildId().empty()) {
    return ErrorMessage(
        absl::StrFormat("Symbols file \"%s\" does not have a build id", symbols_path.string()));
  }
  const std::string& file_build_id = symbols_file->GetBuildId();
  if (build_id != file_build_id) {
    return ErrorMessage(
        absl::StrFormat(R"(Symbols file "%s" has a different build id: "%s" != "%s")",
                        symbols_path.string(), build_id, file_build_id));
  }
  return outcome::success();
}

SymbolHelper::SymbolHelper()
    : symbols_file_directories_(ReadSymbolsFile()), cache_directory_(Path::CreateOrGetCacheDir()) {}

ErrorMessageOr<fs::path> SymbolHelper::FindSymbolsWithSymbolsPathFile(
    const fs::path& module_path, const std::string& build_id) const {
  if (build_id.empty()) {
    return ErrorMessage(absl::StrFormat(
        "Could not find symbols file for module \"%s\", because it does not contain a build id",
        module_path.string()));
  }

  const fs::path& filename = module_path.filename();
  fs::path filename_dot_debug = filename;
  filename_dot_debug.replace_extension(".debug");
  fs::path filename_plus_debug = filename;
  filename_plus_debug.replace_extension(filename.extension().string() + ".debug");

  std::set<fs::path> search_paths;
  for (const auto& directory : symbols_file_directories_) {
    search_paths.insert(directory / filename_dot_debug);
    search_paths.insert(directory / filename_plus_debug);
    search_paths.insert(directory / filename);
  }

  LOG("Trying to find symbols for module: \"%s\"", module_path.string());
  for (const auto& symbols_path : search_paths) {
    if (!fs::exists(symbols_path)) continue;

    const auto verification_result = VerifySymbolsFile(symbols_path, build_id);
    if (!verification_result) {
      LOG("Existing file \"%s\" is not the symbols file for module \"%s\", error: %s",
          symbols_path.string(), module_path.string(), verification_result.error().message());
      continue;
    }

    LOG("Found debug info for module \"%s\" -> \"%s\"", module_path.string(),
        symbols_path.string());
    return symbols_path;
  }

  return ErrorMessage(absl::StrFormat("Could not find a file with debug symbols for module \"%s\"",
                                      module_path.string()));
}

[[nodiscard]] ErrorMessageOr<fs::path> SymbolHelper::FindSymbolsInCache(
    const fs::path& module_path, const std::string& build_id) const {
  fs::path cache_file_path = GenerateCachedFileName(module_path);
  if (!fs::exists(cache_file_path)) {
    return ErrorMessage(
        absl::StrFormat("Unable to find symbols in cache for module \"%s\"", module_path.string()));
  }
  OUTCOME_TRY(VerifySymbolsFile(cache_file_path, build_id));
  return cache_file_path;
}

ErrorMessageOr<ModuleSymbols> SymbolHelper::LoadSymbolsFromFile(const fs::path& file_path) {
  ORBIT_SCOPE_FUNCTION;
  SCOPED_TIMED_LOG("LoadSymbolsFromFile: %s", file_path.string());
  ErrorMessageOr<std::unique_ptr<ElfFile>> elf_file_result = ElfFile::Create(file_path.string());

  if (!elf_file_result) {
    return ErrorMessage(absl::StrFormat("Failed to load debug symbols from \"%s\": %s",
                                        file_path.string(), elf_file_result.error().message()));
  }

  return elf_file_result.value()->LoadSymbols();
}

fs::path SymbolHelper::GenerateCachedFileName(const fs::path& file_path) const {
  auto file_name = absl::StrReplaceAll(file_path.string(), {{"/", "_"}});
  return cache_directory_ / file_name;
}
