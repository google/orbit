// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_PDB_FILE_LLVM_H_
#define OBJECT_UTILS_PDB_FILE_LLVM_H_

#include <llvm/DebugInfo/CodeView/CVSymbolVisitor.h>
#include <llvm/DebugInfo/CodeView/CodeView.h>
#include <llvm/DebugInfo/CodeView/GUID.h>
#include <llvm/DebugInfo/CodeView/SymbolDeserializer.h>
#include <llvm/DebugInfo/CodeView/SymbolVisitorCallbackPipeline.h>
#include <llvm/DebugInfo/CodeView/SymbolVisitorCallbacks.h>
#include <llvm/DebugInfo/MSF/MappedBlockStream.h>
#include <llvm/DebugInfo/PDB/IPDBSession.h>
#include <llvm/DebugInfo/PDB/Native/DbiStream.h>
#include <llvm/DebugInfo/PDB/Native/InfoStream.h>
#include <llvm/DebugInfo/PDB/Native/ModuleDebugStream.h>
#include <llvm/DebugInfo/PDB/Native/NativeSession.h>
#include <llvm/DebugInfo/PDB/Native/PDBFile.h>
#include <llvm/DebugInfo/PDB/PDB.h>
#include <llvm/DebugInfo/PDB/PDBSymbolExe.h>
#include <llvm/DebugInfo/PDB/PDBTypes.h>
#include <llvm/Demangle/Demangle.h>
#include <stdint.h>

#include <array>
#include <filesystem>
#include <memory>
#include <string>

#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/PdbFile.h"
#include "ObjectUtils/SymbolsFile.h"
#include "ObjectUtils/WindowsBuildIdUtils.h"
#include "OrbitBase/Result.h"

namespace orbit_object_utils {

class PdbFileLlvm : public PdbFile {
 public:
  [[nodiscard]] ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadDebugSymbols() override;
  [[nodiscard]] const std::filesystem::path& GetFilePath() const override { return file_path_; }

  [[nodiscard]] std::array<uint8_t, 16> GetGuid() const override;

  [[nodiscard]] uint32_t GetAge() const override;

  [[nodiscard]] std::string GetBuildId() const override {
    return ComputeWindowsBuildId(GetGuid(), GetAge());
  }

  static ErrorMessageOr<std::unique_ptr<PdbFile>> CreatePdbFile(
      const std::filesystem::path& file_path, const ObjectFileInfo& object_file_info);

 private:
  PdbFileLlvm(std::filesystem::path file_path, const ObjectFileInfo& object_file_info,
              std::unique_ptr<llvm::pdb::IPDBSession> session);

  std::filesystem::path file_path_;
  ObjectFileInfo object_file_info_;
  std::unique_ptr<llvm::pdb::IPDBSession> session_;
};

ErrorMessageOr<std::unique_ptr<PdbFile>> CreatePdbFileLlvm(const std::filesystem::path& file_path,
                                                           const ObjectFileInfo& object_file_info);

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_PDB_FILE_LLVM_H_
