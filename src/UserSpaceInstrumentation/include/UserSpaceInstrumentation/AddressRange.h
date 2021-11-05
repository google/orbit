// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_ADDRESS_RANGE_H_
#define USER_SPACE_INSTRUMENTATION_ADDRESS_RANGE_H_

#include <cstdint>
#include <utility>

namespace orbit_user_space_instrumentation {

// Represents an address range in the tracee. Mostly useful in the context of parsing the content of
// "/proc/pid/maps". As in the maps file, the address range is stored using the first and "one past
// last" valid addresses.
struct AddressRange {
  AddressRange() = default;
  AddressRange(uint64_t start, uint64_t end) : start(start), end(end) {}

  uint64_t start;
  uint64_t end;
};

// Make AddressRange hashable so we can use it as a key in a flat_hash_map.
inline bool operator==(const AddressRange& lhs, const AddressRange& rhs) {
  return lhs.start == rhs.start && lhs.end == rhs.end;
}
template <typename H>
H AbslHashValue(H h, const AddressRange& a) {
  return H::combine(std::move(h), a.start, a.end);
}

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_ADDRESS_RANGE_H_