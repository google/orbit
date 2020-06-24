// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MODULE_DATA_H_
#define ORBIT_GL_MODULE_DATA_H_

#include <cinttypes>
#include <memory>
#include <string>
#include <utility>

#include "absl/strings/str_format.h"
#include "module.pb.h"

// Represents information about module on the client
class ModuleData final {
 public:
  explicit ModuleData(ModuleInfo info)
      : module_info_(std::move(info)), is_loaded_(false) {}

  void SetModuleInfo(const ModuleInfo& info) { module_info_ = info; }

  const std::string& name() const { return module_info_.name(); }
  const std::string& file_path() const { return module_info_.file_path(); }
  uint64_t file_size() const { return module_info_.file_size(); }
  uint64_t address_start() const { return module_info_.address_start(); }

  std::string address_range() const {
    return absl::StrFormat("[%016" PRIx64 " - %016" PRIx64 "]",
                           module_info_.address_start(),
                           module_info_.address_end());
  }

  void set_loaded(bool value) { is_loaded_ = value; }
  bool is_loaded() const { return is_loaded_; }

 private:
  ModuleInfo module_info_;
  bool is_loaded_;
};

#endif  // ORBIT_GL_MODULE_DATA_H_
