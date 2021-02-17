// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELF_UTILS_ELF_FILE_H_
#define ELF_UTILS_ELF_FILE_H_

#include <stddef.h>
#include <stdint.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "OrbitBase/Result.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"
#include "symbol.pb.h"

namespace orbit_elf_utils {

struct GnuDebugLinkInfo {
  std::filesystem::path path;

  // That's a CRC32 checksum of the file contents of the separate debuginfo file.
  // Check the ELF documentation for the exact polynomial and the initial value.
  // Docs: https://refspecs.linuxfoundation.org/elf/elf.pdf
  uint32_t crc32_checksum;
};

class ElfFile {
 public:
  ElfFile() = default;
  virtual ~ElfFile() = default;

  [[nodiscard]] virtual ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadSymbols() = 0;
  // Background and some terminology
  // When an elf file is loaded to memory it has its load segments
  // (segments of PT_LOAD type from program headers) mapped to some
  // location in memory. The location of the first segment is called a base
  // address.
  // Symbols addresses in the elf file however are not offsets from the base
  // address, they are calculated as offset from some virtual 0 where that
  // virtual zero can be different from the base address.
  //
  // The way to calculate the virtual zero is to take base address and subtract
  // from it the minimum vaddr specified in PT_LOAD program headers.
  //
  // This method returns load bias for the elf-file if program headers are
  // available. This should be the case for all loadable elf-files.
  [[nodiscard]] virtual ErrorMessageOr<uint64_t> GetLoadBias() const = 0;
  [[nodiscard]] virtual bool HasSymtab() const = 0;
  [[nodiscard]] virtual bool HasDebugInfo() const = 0;
  [[nodiscard]] virtual bool HasGnuDebuglink() const = 0;
  [[nodiscard]] virtual bool Is64Bit() const = 0;
  [[nodiscard]] virtual std::string GetBuildId() const = 0;
  [[nodiscard]] virtual std::filesystem::path GetFilePath() const = 0;
  [[nodiscard]] virtual ErrorMessageOr<orbit_grpc_protos::LineInfo> GetLineInfo(
      uint64_t address) = 0;
  [[nodiscard]] virtual std::optional<GnuDebugLinkInfo> GetGnuDebugLinkInfo() const = 0;

  [[nodiscard]] static ErrorMessageOr<std::unique_ptr<ElfFile>> Create(
      const std::filesystem::path& file_path);
  [[nodiscard]] static ErrorMessageOr<std::unique_ptr<ElfFile>> Create(
      const std::filesystem::path& file_path,
      llvm::object::OwningBinary<llvm::object::ObjectFile>&& file);
  [[nodiscard]] static ErrorMessageOr<std::unique_ptr<ElfFile>> CreateFromBuffer(
      const std::filesystem::path& file_path, const void* buf, size_t len);
};

}  // namespace orbit_elf_utils

#endif  // ELF_UTILS_ELF_FILE_H_
