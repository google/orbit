// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOLS_SYMBOL_HELPER_H_
#define SYMBOLS_SYMBOL_HELPER_H_

#include <absl/types/span.h>
#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>
#include <stdint.h>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/SymbolsFile.h"
#include "OrbitBase/Result.h"
#include "SymbolProvider/StructuredDebugDirectorySymbolProvider.h"
#include "Symbols/SymbolCacheInterface.h"

namespace orbit_symbols {

class SymbolHelper : public SymbolCacheInterface {
 public:
  explicit SymbolHelper(std::filesystem::path cache_directory);
  explicit SymbolHelper(std::filesystem::path cache_directory,
                        absl::Span<const std::filesystem::path> structured_debug_directories);

  [[nodiscard]] ErrorMessageOr<std::filesystem::path> FindSymbolsFileLocally(
      const std::filesystem::path& module_path, std::string_view build_id,
      const orbit_grpc_protos::ModuleInfo::ObjectFileType& object_file_type,
      absl::Span<const std::filesystem::path> paths) const;
  [[nodiscard]] ErrorMessageOr<std::filesystem::path> FindSymbolsInCache(
      const std::filesystem::path& module_path, std::string_view build_id) const;
  [[nodiscard]] ErrorMessageOr<std::filesystem::path> FindSymbolsInCache(
      const std::filesystem::path& module_path, uint64_t expected_file_size) const;
  [[nodiscard]] ErrorMessageOr<std::filesystem::path> FindObjectInCache(
      const std::filesystem::path& module_path, std::string_view build_id,
      uint64_t expected_file_size) const;
  [[nodiscard]] std::filesystem::path GenerateCachedFilePath(
      const std::filesystem::path& file_path) const override;

  static ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadSymbolsFromFile(
      const std::filesystem::path& file_path,
      const orbit_object_utils::ObjectFileInfo& object_file_info);
  static ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadFallbackSymbolsFromFile(
      const std::filesystem::path& file_path);

  [[nodiscard]] static bool IsMatchingDebugInfoFile(const std::filesystem::path& file_path,
                                                    uint32_t checksum);
  [[nodiscard]] static ErrorMessageOr<std::filesystem::path> FindDebugInfoFileLocally(
      std::string_view filename, uint32_t checksum,
      absl::Span<const std::filesystem::path> directories);

 private:
  template <typename Verifier>
  ErrorMessageOr<std::filesystem::path> FindSymbolsInCacheImpl(
      const std::filesystem::path& module_path, std::string_view searchee_for_error_message,
      Verifier&& verify) const;

  const std::filesystem::path cache_directory_;
  // TODO(b/246743231): Move this out of SymbolHelper in a next refactoring step.
  std::vector<orbit_symbol_provider::StructuredDebugDirectorySymbolProvider>
      structured_debug_directory_providers_;
};

[[nodiscard]] std::vector<std::filesystem::path> ReadSymbolsFile(
    const std::filesystem::path& file_name);

ErrorMessageOr<bool> FileStartsWithDeprecationNote(const std::filesystem::path& file_name);

ErrorMessageOr<void> AddDeprecationNoteToFile(const std::filesystem::path& file_name);

}  // namespace orbit_symbols

#endif  // SYMBOLS_SYMBOL_HELPER_H_
