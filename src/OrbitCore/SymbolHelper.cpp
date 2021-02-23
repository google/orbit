// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SymbolHelper.h"

#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>
#include <absl/strings/str_split.h>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <outcome.hpp>
#include <set>
#include <system_error>

#include "ElfUtils/ElfFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/Tracing.h"
#include "OrbitBase/WriteStringToFile.h"
#include "Path.h"

using orbit_grpc_protos::ModuleSymbols;

namespace fs = std::filesystem;
using ::orbit_elf_utils::ElfFile;

namespace {

std::vector<fs::path> ReadSymbolsFile() {
  fs::path file_name = Path::GetSymbolsFileName();
  std::error_code error;
  bool file_exists = fs::exists(file_name, error);
  if (error) {
    ERROR("Unable to stat \"%s\":%s", file_name.string(), error.message());
    return {};
  }

  if (!file_exists) {
    ErrorMessageOr<void> result = orbit_base::WriteStringToFile(
        file_name,
        "//-------------------\n"
        "// Orbit Symbol Locations\n"
        "//-------------------\n"
        "// Orbit will scan the specified directories for symbol files.\n"
        "// Enter one directory per line, like so:\n"
#ifdef _WIN32
        "// C:\\MyApp\\Release\\\n"
        "// D:\\MySymbolServer\\\n"
#else
        "// /home/git/project/build/\n"
        "// /home/symbol_server/\n"
#endif
    );

    if (!result) {
      ERROR("Unable to create symbols file: %s", result.error().message());
    }
    // Since file is empty - return empty list
    return {};
  }

  std::vector<fs::path> directories;
  ErrorMessageOr<std::string> file_content = orbit_base::ReadFileToString(file_name);
  if (!file_content) {
    ERROR("%s", file_content.error().message());
    return {};
  }

  std::vector<std::string> lines = absl::StrSplit(file_content.value(), absl::ByAnyChar("\r\n"));
  for (const std::string& line : lines) {
    if (absl::StartsWith(line, "//") || line.empty()) continue;

    const fs::path& dir = line;
    bool is_directory = fs::is_directory(dir, error);
    if (error) {
      ERROR("Unable to stat \"%s\": %s (skipping)", dir.string(), error.message());
      continue;
    }

    if (!is_directory) {
      ERROR("\"%s\" is not a directory (skipping)", dir.string());
      continue;
    }

    directories.push_back(dir);
  }
  return directories;
}

}  // namespace

ErrorMessageOr<void> SymbolHelper::VerifySymbolsFile(const fs::path& symbols_path,
                                                     const std::string& build_id) {
  OUTCOME_TRY(symbols_file, ElfFile::Create(symbols_path));

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
    std::error_code error;
    bool exists = fs::exists(symbols_path, error);
    if (error) {
      ERROR("Unable to stat \"%s\": %s", symbols_path.string(), error.message());
      continue;
    }

    if (!exists) continue;

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
  std::error_code error;
  bool exists = fs::exists(cache_file_path, error);
  if (error) {
    return ErrorMessage{
        absl::StrFormat("Unable to stat \"%s\": %s", cache_file_path.string(), error.message())};
  }
  if (!exists) {
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

[[nodiscard]] bool SymbolHelper::IsMatchingDebugInfoFile(
    const std::filesystem::path& debuginfo_file_path, uint32_t checksum) {
  std::error_code error;
  bool exists = fs::exists(debuginfo_file_path, error);
  if (error) {
    ERROR("Unable to stat \"%s\": %s", debuginfo_file_path.string(), error.message());
    return false;
  }

  if (!exists) return false;

  const auto checksum_or_error = ElfFile::CalculateDebuglinkChecksum(debuginfo_file_path);
  if (checksum_or_error.has_error()) {
    LOG("Unable to calculate checksum of \"%s\": \"%s\"", debuginfo_file_path.filename().string(),
        checksum_or_error.error().message());
    return false;
  }

  if (checksum_or_error.value() != checksum) {
    LOG("Found file with matching name \"%s\", but the checksums do not match. Expected: %#x. "
        "Actual: %#x",
        debuginfo_file_path.string(), checksum, checksum_or_error.value());
    return false;
  }

  LOG("Found debug info in file \"%s\"", debuginfo_file_path.string());
  return true;
}

ErrorMessageOr<fs::path> SymbolHelper::FindDebugInfoFileLocally(std::string_view filename,
                                                                uint32_t checksum) const {
  std::set<fs::path> search_paths;
  for (const auto& directory : symbols_file_directories_) {
    search_paths.insert(directory / filename);
  }

  LOG("Trying to find debuginfo file with filename \"%s\"", filename);
  for (const auto& debuginfo_file_path : search_paths) {
    if (IsMatchingDebugInfoFile(debuginfo_file_path, checksum)) return debuginfo_file_path;
  }

  return ErrorMessage{
      absl::StrFormat("Could not find a file with debug info with filename \"%s\" and checksum %#x",
                      filename, checksum)};
}

ErrorMessageOr<fs::path> SymbolHelper::FindDebugInfoFileInDebugStore(
    const fs::path& debug_directory, std::string_view build_id) {
  if (build_id.size() < 3) {
    return ErrorMessage{absl::StrFormat("The build-id \"%s\" is malformed.", build_id)};
  }

  auto full_file_path = debug_directory / ".build-id" / build_id.substr(0, 2) /
                        absl::StrFormat("%s.debug", build_id.substr(2));

  std::error_code error{};
  if (fs::exists(full_file_path, error)) return full_file_path;

  if (error.value() == 0) {
    return ErrorMessage{absl::StrFormat("File does not exist: \"%s\"", full_file_path.string())};
  }

  return ErrorMessage{absl::StrFormat("Unable to stat the file \"%s\": %s", full_file_path.string(),
                                      error.message())};
}