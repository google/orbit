// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_COFF_FILE_H_
#define OBJECT_UTILS_COFF_FILE_H_

#include <filesystem>
#include <memory>

#include "ObjectUtils/ObjectFile.h"
#include "OrbitBase/Result.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"
#include "symbol.pb.h"

namespace orbit_object_utils {

class CoffFile : public ObjectFile {
 public:
  CoffFile() = default;
  virtual ~CoffFile() = default;
};

[[nodiscard]] ErrorMessageOr<std::unique_ptr<CoffFile>> CreateCoffFile(
    const std::filesystem::path& file_path);
[[nodiscard]] ErrorMessageOr<std::unique_ptr<CoffFile>> CreateCoffFile(
    const std::filesystem::path& file_path,
    llvm::object::OwningBinary<llvm::object::ObjectFile>&& file);

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_COFF_FILE_H_
