// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_DATA_SCOPE_INFO_H_
#define ORBIT_CLIENT_DATA_SCOPE_INFO_H_

#include <string>

namespace orbit_client_data {

enum ScopeType : uint8_t { kOther = 0, kHookedFunction = 1, kApiScope = 2, kApiScopeAsync = 3 };

struct ScopeInfo {
  template <typename H>
  friend H AbslHashValue(H h, const ScopeInfo& scope) {
    return H::combine(std::move(h), scope.name, scope.type);
  }

  friend bool operator==(const ScopeInfo& lhs, const ScopeInfo& rhs) {
    return lhs.type == rhs.type && lhs.name == rhs.name;
  }

  std::string name;
  ScopeType type{};
};

}  // namespace orbit_client_data

#endif  // ORBIT_CLIENT_DATA_SCOPE_INFO_H_