// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_LIST_MODULES_H_
#define WINDOWS_UTILS_LIST_MODULES_H_

#include <string>
#include <vector>

#include "GrpcProtos/module.pb.h"

namespace orbit_windows_utils {

struct Module {
  std::string name;
  std::string full_path;
  uint64_t file_size = 0;
  uint64_t address_start = 0;
  uint64_t address_end = 0;
  uint64_t load_bias = 0;
  std::string build_id;
  std::vector<orbit_grpc_protos::ModuleInfo::ObjectSegment> sections;
};

// List all modules of the process identified by "pid".
[[nodiscard]] std::vector<Module> ListModules(uint32_t pid);

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_LIST_MODULES_H_
