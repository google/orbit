// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>

#include <array>
#include <cstdint>
#include <string>

namespace orbit_object_utils {

std::string ComputeWindowsBuildId(std::array<uint8_t, 16> guid, uint32_t age) {
  std::string build_id;
  for (const uint8_t& byte : guid) {
    absl::StrAppend(&build_id, absl::Hex(byte, absl::kZeroPad2));
  }

  // The dash ("-") is intentional to make it easy to distinguish the age when debugging issues
  // related to build id.
  absl::StrAppend(&build_id, absl::StrFormat("-%i", age));
  return build_id;
}

}  // namespace orbit_object_utils