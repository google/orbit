// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_MODULE_IDENTIFIER_H_
#define CLIENT_DATA_MODULE_IDENTIFIER_H_

#include <string>
#include <tuple>

#include "OrbitBase/Typedef.h"

namespace orbit_client_data {

// This class is used to uniquely identify a module, e.g. to store the identifier into a map.
// `ModuleIdentifier`s can only be created by a `ModuleIdentifierProvider`, which also ensures
// uniqueness of ids for the same modules.
class ModuleIdentifier {
  friend class ModuleIdentifierProvider;

  [[nodiscard]] friend bool operator==(const ModuleIdentifier& lhs, const ModuleIdentifier& rhs) {
    return lhs.module_identifier_ == rhs.module_identifier_;
  }
  [[nodiscard]] friend bool operator!=(const ModuleIdentifier& lhs, const ModuleIdentifier& rhs) {
    return !(lhs == rhs);
  }

  template <typename H>
  friend H AbslHashValue(H h, const ModuleIdentifier& o) {
    return H::combine(std::move(h), o.module_identifier_);
  }

 private:
  explicit ModuleIdentifier(uint64_t module_identifier) : module_identifier_{module_identifier} {}

  uint64_t module_identifier_;
};

}  // namespace orbit_client_data

#endif  // SYMBOL_PROVIDER_MODULE_IDENTIFIER_H_
