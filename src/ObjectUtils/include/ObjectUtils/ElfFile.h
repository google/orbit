// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_ELF_FILE_H_
#define OBJECT_UTILS_ELF_FILE_H_

#include <stddef.h>
#include <stdint.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ObjectFile.h"
#include "OrbitBase/Result.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"
#include "symbol.pb.h"

namespace orbit_object_utils {

struct GnuDebugLinkInfo {
  std::filesystem::path path;

  // That's a CRC32 checksum of the file contents of the separate debuginfo file.
  // Check the ELF documentation for the exact polynomial and the initial value.
  // Docs: https://refspecs.linuxfoundation.org/elf/elf.pdf
  uint32_t crc32_checksum;
};

class ElfFile : public ObjectFile {
 public:
  ElfFile() = default;
  virtual ~ElfFile() = default;

  [[nodiscard]] virtual ErrorMessageOr<orbit_grpc_protos::ModuleSymbols>
  LoadSymbolsFromDynsym() = 0;

  [[nodiscard]] virtual bool HasDynsym() const = 0;
  [[nodiscard]] virtual bool HasDebugInfo() const = 0;
  [[nodiscard]] virtual bool HasGnuDebuglink() const = 0;
  [[nodiscard]] virtual bool Is64Bit() const = 0;
  [[nodiscard]] virtual std::string GetSoname() const = 0;
  [[nodiscard]] virtual std::string GetBuildId() const = 0;
  [[nodiscard]] virtual ErrorMessageOr<orbit_grpc_protos::LineInfo> GetLineInfo(
      uint64_t address) = 0;
  [[nodiscard]] virtual ErrorMessageOr<orbit_grpc_protos::LineInfo>
  GetDeclarationLocationOfFunction(uint64_t address) = 0;
  [[nodiscard]] virtual std::optional<GnuDebugLinkInfo> GetGnuDebugLinkInfo() const = 0;

  [[nodiscard]] static ErrorMessageOr<uint32_t> CalculateDebuglinkChecksum(
      const std::filesystem::path& file_path);
};

[[nodiscard]] ErrorMessageOr<std::unique_ptr<ElfFile>> CreateElfFile(
    const std::filesystem::path& file_path);
[[nodiscard]] ErrorMessageOr<std::unique_ptr<ElfFile>> CreateElfFile(
    const std::filesystem::path& file_path,
    llvm::object::OwningBinary<llvm::object::ObjectFile>&& file);
[[nodiscard]] ErrorMessageOr<std::unique_ptr<ElfFile>> CreateElfFileFromBuffer(
    const std::filesystem::path& file_path, const void* buf, size_t len);

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_ELF_FILE_H_
