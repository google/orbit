// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOL_PROVIDER_MODULE_PATH_AND_BUILD_ID_H_
#define SYMBOL_PROVIDER_MODULE_PATH_AND_BUILD_ID_H_

#include <string>

namespace orbit_client_data {

struct ModulePathAndBuildId {
  std::string module_path;
  std::string build_id;

  [[nodiscard]] friend bool operator==(const ModulePathAndBuildId& lhs,
                                       const ModulePathAndBuildId& rhs) {
    return lhs.module_path == rhs.module_path && lhs.build_id == rhs.build_id;
  }
  [[nodiscard]] friend bool operator!=(const ModulePathAndBuildId& lhs,
                                       const ModulePathAndBuildId& rhs) {
    return !(lhs == rhs);
  }

  template <typename H>
  friend H AbslHashValue(H h, const ModulePathAndBuildId& o) {
    return H::combine(std::move(h), o.module_path, o.build_id);
  }
};

}  // namespace orbit_client_data

#endif  // SYMBOL_PROVIDER_MODULE_PATH_AND_BUILD_ID_H_
