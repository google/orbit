// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ObjectUtils/ObjectFile.h"

#include <absl/strings/str_format.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/Error.h>

#include <filesystem>
#include <memory>
#include <string>
#include <utility>

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
    return ErrorMessage(absl::StrFormat("Unable to load object file \"%s\": %s.",
                                        file_path.string(),
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

}  // namespace orbit_object_utils