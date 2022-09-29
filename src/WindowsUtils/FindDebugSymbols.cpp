// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/FindDebugSymbols.h"

#include <absl/base/casts.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>
#include <windows.h>

#include <filesystem>
#include <optional>

#include "ObjectUtils/ObjectFile.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"

namespace orbit_windows_utils {

using orbit_object_utils::CreateObjectFile;
using orbit_object_utils::CreateSymbolsFile;
using orbit_object_utils::ObjectFile;
using orbit_object_utils::SymbolsFile;

ErrorMessageOr<std::filesystem::path> FindDebugSymbols(
    std::filesystem::path module_path,
    std::vector<std::filesystem::path> additional_search_directories) {
  // Create object file for the module.
  OUTCOME_TRY(std::unique_ptr<ObjectFile> object_file, CreateObjectFile(module_path));

  // If module does not contain a build id, no searching for separate symbol files can be done
  if (object_file->GetBuildId().empty()) {
    return ErrorMessage{
        absl::StrFormat("Module \"%s\" does not contain symbols and does not contain a build id, "
                        "therefore Orbit cannot search for a separate symbols file",
                        module_path.string())};
  }

  // Coff is currently the only supported format for the Windows service.
  if (!object_file->IsCoff()) {
    return ErrorMessage{
        absl::StrFormat("Module \"%s\" is not of Coff file format, which is currently the only "
                        "supported format for the Windows service.",
                        module_path.string())};
  }

  const std::string& build_id = object_file->GetBuildId();

  // Search in module's directory and in the user provided additional directories.
  std::vector<std::filesystem::path> search_directories{object_file->GetFilePath().parent_path()};
  search_directories.insert(search_directories.end(), additional_search_directories.begin(),
                            additional_search_directories.end());

  // Files with the following formats are considered: "module.sym_ext" and "module.mod_ext.sym_ext".
  // "mod_ext" is the module file extension, usually ".exe" or ".dll"; "sym_ext" is ".pdb".
  const std::string sym_ext = ".pdb";
  const std::filesystem::path& filename = module_path.filename();
  std::filesystem::path filename_dot_sym_ext = filename;
  filename_dot_sym_ext.replace_extension(sym_ext);
  std::filesystem::path filename_plus_sym_ext = filename;
  filename_plus_sym_ext.replace_extension(filename.extension().string() + sym_ext);

  std::set<std::filesystem::path> search_paths;
  for (const auto& directory : search_directories) {
    search_paths.insert(directory / filename_dot_sym_ext);
    search_paths.insert(directory / filename_plus_sym_ext);
  }

  std::vector<std::string> error_messages;

  for (const auto& search_path : search_paths) {
    ErrorMessageOr<bool> file_exists = orbit_base::FileOrDirectoryExists(search_path);

    if (file_exists.has_error()) {
      ORBIT_ERROR("%s", file_exists.error().message());
      error_messages.emplace_back(file_exists.error().message());
      continue;
    }

    if (!file_exists.value()) {
      // no error message when file simply does not exist
      continue;
    }

    orbit_object_utils::ObjectFileInfo object_file_info{object_file->GetLoadBias()};

    ErrorMessageOr<std::unique_ptr<SymbolsFile>> symbols_file_or_error =
        CreateSymbolsFile(search_path, object_file_info);

    if (symbols_file_or_error.has_error()) {
      error_messages.push_back(symbols_file_or_error.error().message());
      continue;
    }
    const std::unique_ptr<SymbolsFile>& symbols_file{symbols_file_or_error.value()};

    if (symbols_file->GetBuildId().empty()) {
      error_messages.push_back(absl::StrFormat(
          "Potential symbols file \"%s\" does not have a build id.", search_path.string()));
      continue;
    }

    if (symbols_file->GetBuildId() != build_id) {
      error_messages.push_back(absl::StrFormat(
          "Potential symbols file \"%s\" has a different build id than the module requested by "
          "the client: \"%s\" != \"%s\"",
          search_path.string(), symbols_file->GetBuildId(), build_id));
      continue;
    }

    return search_path;
  }

  std::string error_message_for_client{absl::StrFormat(
      "Unable to find debug symbols on the instance for module \"%s\".", module_path.string())};
  if (!error_messages.empty()) {
    absl::StrAppend(&error_message_for_client, ":\n  * ", absl::StrJoin(error_messages, "\n  * "));
  }

  return ErrorMessage(error_message_for_client);
}

}  // namespace orbit_windows_utils
