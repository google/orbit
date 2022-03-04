// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ObjectUtils/PdbFile.h"

#if _WIN32
#include "PdbFileDia.h"
#endif

#include "PdbFileLlvm.h"

namespace orbit_object_utils {

ErrorMessageOr<std::unique_ptr<PdbFile>> CreatePdbFile(const std::filesystem::path& file_path,
                                                       const ObjectFileInfo& object_file_info) {
#if _WIN32
  // To workaround a limitation in LLVM's pdb parsing code, we use the DIA SDK directly on Windows.
  return PdbFileDia::CreatePdbFile(file_path, object_file_info);
#else
  return PdbFileLlvm::CreatePdbFile(file_path, object_file_info);
#endif
}

#if _WIN32
ErrorMessageOr<std::unique_ptr<PdbFile>> CreatePdbFile(const std::filesystem::path& file_path,
                                                       const ObjectFileInfo& object_file_info,
                                                       PdbParserType parser_type) {
  switch (parser_type) {
    case PdbParserType::Llvm:
      return PdbFileLlvm::CreatePdbFile(file_path, object_file_info);
    case PdbParserType::Dia:
      return PdbFileDia::CreatePdbFile(file_path, object_file_info);
  }

  return ErrorMessage("Unhandled parser type");
}
#endif

}  // namespace orbit_object_utils
