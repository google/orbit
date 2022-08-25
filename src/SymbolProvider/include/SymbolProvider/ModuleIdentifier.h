// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOL_PROVIDER_MODULE_IDENTIFIER_H_
#define SYMBOL_PROVIDER_MODULE_IDENTIFIER_H_

#include <string>
#include <tuple>

namespace orbit_symbol_provider {

struct ModuleIdentifier {
  // TODO (b/241203463) Consider changing the type to std::filesystem::path
  std::string file_path;
  std::string build_id;

  friend bool operator==(const ModuleIdentifier& lhs, const ModuleIdentifier& rhs) {
    return std::tie(lhs.file_path, lhs.build_id) == std::tie(rhs.file_path, rhs.build_id);
  }
  friend bool operator!=(const ModuleIdentifier& lhs, const ModuleIdentifier& rhs) {
    return !(lhs == rhs);
  }
  template <typename H>
  friend H AbslHashValue(H h, const ModuleIdentifier& id) {
    return H::combine(std::move(h), id.file_path, id.build_id);
  }
};

}  // namespace orbit_symbol_provider

#endif  // SYMBOL_PROVIDER_MODULE_IDENTIFIER_H_
