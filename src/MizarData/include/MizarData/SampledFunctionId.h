// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_SAMPLED_FUNCTION_ID_H_
#define MIZAR_DATA_SAMPLED_FUNCTION_ID_H_

#include <absl/strings/str_format.h>
#include <stdint.h>

#include <string>
#include <utility>

namespace orbit_mizar_data {

// The class represents a sampled function id. These ids are the same for the same function across
// all the captures.
class SFID {
 public:
  constexpr explicit SFID(uint64_t id) : id_(id) {}

  friend bool operator==(const SFID& lhs, const SFID& rhs) { return lhs.id_ == rhs.id_; }

  template <typename H>
  friend H AbslHashValue(H h, const SFID& c) {
    return H::combine(std::move(h), c.id_);
  }

  // For debug purposes
  explicit operator std::string() const { return std::to_string(id_); }

 private:
  uint64_t id_;
};

// Making sure we do not waste memory on the abstraction
static_assert(sizeof(SFID) == sizeof(uint64_t));

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_SAMPLED_FUNCTION_ID_H_
