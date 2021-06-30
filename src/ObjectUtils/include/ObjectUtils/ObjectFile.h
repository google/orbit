// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_OBJECT_FILE_H_
#define OBJECT_UTILS_OBJECT_FILE_H_

#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>

#include <filesystem>
#include <memory>
#include <string>

#include "OrbitBase/Result.h"
#include "symbol.pb.h"

namespace orbit_object_utils {

class ObjectFile {
 public:
  ObjectFile() = default;
  virtual ~ObjectFile() = default;

  [[nodiscard]] virtual ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadDebugSymbols() = 0;
  [[nodiscard]] virtual bool HasDebugSymbols() const = 0;
  [[nodiscard]] virtual std::string GetName() const = 0;
  [[nodiscard]] virtual const std::filesystem::path& GetFilePath() const = 0;

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
  //
  // The same calculation applies for PE/COFF object files. Here, the address
  // to be subtracted is ImageBase as specified in the PE header.
  [[nodiscard]] virtual uint64_t GetLoadBias() const = 0;
  [[nodiscard]] virtual uint64_t GetExecutableSegmentOffset() const = 0;
  [[nodiscard]] virtual bool IsElf() const = 0;
  [[nodiscard]] virtual bool IsCoff() const = 0;
};

ErrorMessageOr<std::unique_ptr<ObjectFile>> CreateObjectFile(
    const std::filesystem::path& file_path);

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_OBJECT_FILE_H_