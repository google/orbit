// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ObjectUtils/ObjectFile.h"

#include <absl/strings/str_format.h>
#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>

#include <filesystem>
#include <memory>
#include <string>

#include "GrpcProtos/symbol.pb.h"
#include "Introspection/Introspection.h"
#include "ObjectUtils/CoffFile.h"
#include "ObjectUtils/ElfFile.h"
#include "OrbitBase/Result.h"

namespace orbit_object_utils {

ErrorMessageOr<std::unique_ptr<ObjectFile>> CreateObjectFile(
    const std::filesystem::path& file_path) {
  ORBIT_SCOPE_FUNCTION;
  // TODO(hebecker): Remove this explicit construction of StringRef when we switch to LLVM10.
  const std::string file_path_str = file_path.string();
  const llvm::StringRef file_path_llvm{file_path_str};

  llvm::Expected<llvm::object::OwningBinary<llvm::object::ObjectFile>> object_file_or_error =
      llvm::object::ObjectFile::createObjectFile(file_path_llvm);

  if (!object_file_or_error) {
    return ErrorMessage(absl::StrFormat("Unable to load ELF file \"%s\": %s", file_path.string(),
                                        llvm::toString(object_file_or_error.takeError())));
  }

  llvm::object::OwningBinary<llvm::object::ObjectFile>& file = object_file_or_error.get();

  if (file.getBinary()->isELF()) {
    auto elf_file_or_error = CreateElfFile(file_path, std::move(file));
    if (elf_file_or_error.has_error()) {
      return ErrorMessage(absl::StrFormat("Unable to load object file as ELF file: %s",
                                          elf_file_or_error.error().message()));
    }
    return std::move(elf_file_or_error.value());
  }
  if (file.getBinary()->isCOFF()) {
    auto coff_file_or_error = CreateCoffFile(file_path, std::move(file));
    if (coff_file_or_error.has_error()) {
      return ErrorMessage(absl::StrFormat("Unable to load object file as COFF file: %s",
                                          coff_file_or_error.error().message()));
    }
    return std::move(coff_file_or_error.value());
  }

  return ErrorMessage("Unknown object file type.");
}

// Comparator to sort SymbolInfos by address, and perform the corresponding binary searches.
bool ObjectFile::SymbolInfoLessByAddress(const orbit_grpc_protos::SymbolInfo& lhs,
                                         const orbit_grpc_protos::SymbolInfo& rhs) {
  return lhs.address() < rhs.address();
}

void ObjectFile::DeduceDebugSymbolMissingSizes(
    std::vector<orbit_grpc_protos::SymbolInfo>* symbol_infos) {
  // We don't have sizes for functions obtained from the COFF symbol table. For these, compute the
  // size as the distance from the address of the next function.
  std::sort(symbol_infos->begin(), symbol_infos->end(), &SymbolInfoLessByAddress);

  for (size_t i = 0; i < symbol_infos->size(); ++i) {
    orbit_grpc_protos::SymbolInfo& symbol_info = symbol_infos->at(i);
    if (symbol_info.size() != kUnknownSymbolSize) {
      // This function symbol was from DWARF debug info and already has a size.
      continue;
    }

    if (i < symbol_infos->size() - 1) {
      // Deduce the size as the distance from the next function's address.
      symbol_info.set_size(symbol_infos->at(i + 1).address() - symbol_info.address());
    } else {
      // If the last symbol doesn't have a size, we can't deduce it, and we just set it to zero.
      symbol_info.set_size(0);
    }
  }
}

}  // namespace orbit_object_utils