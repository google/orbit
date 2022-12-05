// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Symbols/SymbolUtils.h"

#include <absl/strings/str_format.h>

#include <functional>
#include <memory>
#include <string>

#include "ObjectUtils/ObjectFile.h"
#include "ObjectUtils/SymbolsFile.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"

namespace orbit_symbols {

[[nodiscard]] std::vector<std::filesystem::path> GetStandardSymbolFilenamesForModule(
    const std::filesystem::path& module_path,
    const orbit_grpc_protos::ModuleInfo::ObjectFileType& object_file_type) {
  std::string sym_ext;
  switch (object_file_type) {
    case orbit_grpc_protos::ModuleInfo::kElfFile:
      sym_ext = ".debug";
      break;
    case orbit_grpc_protos::ModuleInfo::kCoffFile:
      sym_ext = ".pdb";
      break;
    case orbit_grpc_protos::ModuleInfo::kUnknown:
      ORBIT_ERROR("Unknown object file type");
      return {module_path.filename()};
    case orbit_grpc_protos::
        ModuleInfo_ObjectFileType_ModuleInfo_ObjectFileType_INT_MIN_SENTINEL_DO_NOT_USE_:
      [[fallthrough]];
    case orbit_grpc_protos::
        ModuleInfo_ObjectFileType_ModuleInfo_ObjectFileType_INT_MAX_SENTINEL_DO_NOT_USE_:
      ORBIT_UNREACHABLE();
      break;
  }

  const std::filesystem::path& filename = module_path.filename();
  std::filesystem::path filename_dot_sym_ext = filename;
  filename_dot_sym_ext.replace_extension(sym_ext);
  std::filesystem::path filename_plus_sym_ext = filename;
  filename_plus_sym_ext.replace_extension(filename.extension().string() + sym_ext);

  return {filename_dot_sym_ext, filename_plus_sym_ext, filename};
}

template <typename FileT>
ErrorMessageOr<void> VerifySymbolOrObjectFileWithBuildId(
    const std::filesystem::path& symbols_or_object_path, std::string_view build_id,
    const std::function<ErrorMessageOr<std::unique_ptr<FileT>>(const std::filesystem::path&)>&
        create_symbols_or_object_file) {
  auto symbols_or_object_file_or_error = create_symbols_or_object_file(symbols_or_object_path);
  if (symbols_or_object_file_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat("Unable to load symbols or object file \"%s\": %s",
                                        symbols_or_object_path.string(),
                                        symbols_or_object_file_or_error.error().message()));
  }

  const std::unique_ptr<FileT>& symbols_or_object_file{symbols_or_object_file_or_error.value()};

  if (symbols_or_object_file->GetBuildId().empty()) {
    return ErrorMessage(absl::StrFormat("Symbols or object file \"%s\" does not have a build id.",
                                        symbols_or_object_path.string()));
  }

  if (symbols_or_object_file->GetBuildId() != build_id) {
    return ErrorMessage(absl::StrFormat(
        R"(Symbols or object file "%s" has a different build id: "%s" != "%s")",
        symbols_or_object_path.string(), build_id, symbols_or_object_file->GetBuildId()));
  }

  return outcome::success();
}

static ErrorMessageOr<void> VerifyFileSize(const std::filesystem::path& symbols_or_object_path,
                                           uint64_t expected_file_size) {
  OUTCOME_TRY(const uint64_t actual_file_size, orbit_base::FileSize(symbols_or_object_path));
  if (actual_file_size != expected_file_size) {
    return ErrorMessage(absl::StrFormat("File size doesn't match. Expected: %u, Actual: %u",
                                        expected_file_size, actual_file_size));
  }
  return outcome::success();
}

ErrorMessageOr<void> VerifySymbolFile(const std::filesystem::path& symbol_file_path,
                                      std::string_view build_id) {
  return VerifySymbolOrObjectFileWithBuildId<orbit_object_utils::SymbolsFile>(
      symbol_file_path, build_id, [](const std::filesystem::path& symbols_path) {
        return CreateSymbolsFile(symbols_path, orbit_object_utils::ObjectFileInfo{});
      });
}

ErrorMessageOr<void> VerifySymbolFile(const std::filesystem::path& symbol_file_path,
                                      uint64_t expected_file_size) {
  if (auto symbols_file_or_error =
          CreateSymbolsFile(symbol_file_path, orbit_object_utils::ObjectFileInfo{});
      symbols_file_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat("Unable to load symbols file \"%s\": %s",
                                        symbol_file_path.string(),
                                        symbols_file_or_error.error().message()));
  }

  return VerifyFileSize(symbol_file_path, expected_file_size);
}

ErrorMessageOr<void> VerifyObjectFile(const std::filesystem::path& object_file_path,
                                      std::string_view build_id, uint64_t expected_file_size) {
  OUTCOME_TRY(VerifySymbolOrObjectFileWithBuildId<orbit_object_utils::ObjectFile>(
      object_file_path, build_id, [](const std::filesystem::path& object_path) {
        return orbit_object_utils::CreateObjectFile(object_path);
      }));
  return VerifyFileSize(object_file_path, expected_file_size);
}

}  // namespace orbit_symbols