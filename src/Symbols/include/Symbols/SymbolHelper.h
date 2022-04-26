// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOLS_SYMBOL_HELPER_H_
#define SYMBOLS_SYMBOL_HELPER_H_

#include <absl/types/span.h>
#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/SymbolsFile.h"
#include "OrbitBase/Result.h"

namespace orbit_symbols {

class SymbolHelper {
 public:
  explicit SymbolHelper(std::filesystem::path cache_directory);
  explicit SymbolHelper(std::filesystem::path cache_directory,
                        std::vector<std::filesystem::path> structured_debug_directories)
      : cache_directory_(std::move(cache_directory)),
        structured_debug_directories_{std::move(structured_debug_directories)} {};

  [[nodiscard]] ErrorMessageOr<std::filesystem::path> FindSymbolsFileLocally(
      const std::filesystem::path& module_path, const std::string& build_id,
      const orbit_grpc_protos::ModuleInfo::ObjectFileType& object_file_type,
      absl::Span<const std::filesystem::path> paths) const;
  [[nodiscard]] ErrorMessageOr<std::filesystem::path> FindSymbolsInCache(
      const std::filesystem::path& module_path, const std::string& build_id) const;
  [[nodiscard]] static ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadSymbolsFromFile(
      const std::filesystem::path& file_path,
      const orbit_object_utils::ObjectFileInfo& object_file_info);
  [[nodiscard]] static ErrorMessageOr<void> VerifySymbolsFile(
      const std::filesystem::path& symbols_path, const std::string& build_id);

  [[nodiscard]] std::filesystem::path GenerateCachedFileName(
      const std::filesystem::path& file_path) const;

  [[nodiscard]] static bool IsMatchingDebugInfoFile(const std::filesystem::path& file_path,
                                                    uint32_t checksum);
  [[nodiscard]] ErrorMessageOr<std::filesystem::path> FindDebugInfoFileLocally(
      std::string_view filename, uint32_t checksum,
      absl::Span<const std::filesystem::path> directories) const;

  // Check out GDB's documentation for how a debug directory is structured:
  // https://sourceware.org/gdb/onlinedocs/gdb/Separate-Debug-Files.html
  [[nodiscard]] static ErrorMessageOr<std::filesystem::path> FindDebugInfoFileInDebugStore(
      const std::filesystem::path& debug_directory, std::string_view build_id);

 private:
  const std::filesystem::path cache_directory_;
  const std::vector<std::filesystem::path> structured_debug_directories_;
};

[[nodiscard]] std::vector<std::filesystem::path> ReadSymbolsFile(
    const std::filesystem::path& file_name);

ErrorMessageOr<bool> FileStartsWithDeprecationNote(const std::filesystem::path& file_name);

ErrorMessageOr<void> AddDeprecationNoteToFile(const std::filesystem::path& file_name);

}  // namespace orbit_symbols

#endif  // SYMBOLS_SYMBOL_HELPER_H_
