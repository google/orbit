// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_PDB_FILE_DIA_H_
#define OBJECT_UTILS_PDB_FILE_DIA_H_

#include <Windows.h>
#include <atlbase.h>
#include <cguid.h>
#include <dia2.h>

#include <array>
#include <filesystem>

#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/PdbFile.h"
#include "ObjectUtils/SymbolsFile.h"
#include "ObjectUtils/WindowsBuildIdUtils.h"
#include "OrbitBase/Result.h"

namespace orbit_object_utils {

class PdbFileDia : public PdbFile {
 public:
  ErrorMessageOr<DebugSymbols> LoadDebugSymbols() override;

  [[nodiscard]] const std::filesystem::path& GetFilePath() const override { return file_path_; }

  [[nodiscard]] std::array<uint8_t, 16> GetGuid() const override { return guid_; }
  [[nodiscard]] uint32_t GetAge() const override { return age_; }
  [[nodiscard]] std::string GetBuildId() const override {
    return ComputeWindowsBuildId(GetGuid(), GetAge());
  }

  static ErrorMessageOr<std::unique_ptr<PdbFile>> CreatePdbFile(
      const std::filesystem::path& file_path, const ObjectFileInfo& object_file_info);

 private:
  ErrorMessageOr<void> LoadProcSymbols(const enum SymTagEnum symbol_tag,
                                       DebugSymbols& debug_symbols);
  PdbFileDia(std::filesystem::path file_path, const ObjectFileInfo& object_file_info);
  ErrorMessageOr<void> LoadDataForPDB();
  ErrorMessageOr<CComPtr<IDiaDataSource>> CreateDiaDataSource();

  std::filesystem::path file_path_;
  ObjectFileInfo object_file_info_;

  // Every CoInitialize() call that succeeds with S_OK or S_FALSE needs to have a corresponding
  // CoUninitialize().
  struct ComInitializer {
    ComInitializer() : result(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)) {}
    ~ComInitializer() {
      if (SUCCEEDED(result)) {
        CoUninitialize();
      }
    }
    HRESULT result;
  };

  // This needs to be declared before any COM objects (such as CComPtr) to make sure this
  // is destructed last, as it calls CoUninitialize() if initialization succeeded.
  ComInitializer com_initializer_;
  CComPtr<IDiaDataSource> dia_data_source_ = nullptr;
  CComPtr<IDiaSession> dia_session_ = nullptr;
  CComPtr<IDiaSymbol> dia_global_scope_symbol_ = nullptr;

  uint32_t age_ = 0;
  std::array<uint8_t, 16> guid_;
};

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_PDB_FILE_DIA_H_
