// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Symbols/SymbolUtils.h"

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

}  // namespace orbit_symbols