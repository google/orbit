// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_OBJECT_FILE_H_
#define OBJECT_UTILS_OBJECT_FILE_H_

#include <llvm/Object/ObjectFile.h>

#include <filesystem>
#include <memory>
#include <string>

#include "OrbitBase/Result.h"
#include "llvm/Object/Binary.h"
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
};

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_OBJECT_FILE_H_