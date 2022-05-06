// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_DATA_SCOPE_INFO_H_
#define ORBIT_CLIENT_DATA_SCOPE_INFO_H_

#include <absl/container/flat_hash_set.h>

#include <string>

namespace orbit_client_data {

enum class ScopeType {
  kInvalid = 0,
  kDynamicallyInstrumentedFunction = 1,
  kApiScope = 2,
  kApiScopeAsync = 3
};

const absl::flat_hash_set<ScopeType> kAllValidScopeTypes = {
    ScopeType::kApiScope, ScopeType::kApiScopeAsync, ScopeType::kDynamicallyInstrumentedFunction};

// An instance of the type uniquely identifies a scope by its name and type. The type is hashable
// and implements `==` and `!=` operators.
//
// DO NOT ADD FIELDS TO THE CLASS. Especially, if they don't make sense for all the `ScopeType`s.
// If you really have to, consider using std::variant or inheritance and removing ScopeType.
class ScopeInfo {
 public:
  ScopeInfo(std::string name, ScopeType type) : name_(std::move(name)), type_(type) {}

  [[nodiscard]] const std::string& GetName() const { return name_; }
  [[nodiscard]] ScopeType GetType() const { return type_; }

  template <typename H>
  friend H AbslHashValue(H h, const ScopeInfo& scope) {
    return H::combine(std::move(h), scope.GetName(), scope.GetType());
  }

  friend bool operator==(const ScopeInfo& lhs, const ScopeInfo& rhs) {
    return lhs.GetType() == rhs.GetType() && lhs.GetName() == rhs.GetName();
  }

  friend bool operator!=(const ScopeInfo& lhs, const ScopeInfo& rhs) { return !(lhs == rhs); }

 private:
  std::string name_;
  ScopeType type_;
};

}  // namespace orbit_client_data

#endif  // ORBIT_CLIENT_DATA_SCOPE_INFO_H_