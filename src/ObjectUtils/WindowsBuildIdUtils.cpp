// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <stddef.h>

#include <array>
#include <cstdint>
#include <string>

namespace orbit_object_utils {

std::string ComputeWindowsBuildId(std::array<uint8_t, 16> guid, uint32_t age) {
  // We need to shuffle the first 8 bytes of the guid in order to generate a build id that matches
  // the one we get from dumpbin.exe. It is also the format expected by the Microsoft Symbol Server.
  // { b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15 } becomes:
  // { b3, b2, b1, b0, b5, b4, b7, b6, b8, b9, b10, b11, b12, b13, b14, b15 }.
  constexpr std::array<uint8_t, 8> kIndices = {3, 2, 1, 0, 5, 4, 7, 6};
  std::array<uint8_t, 16> shuffled_guid = guid;
  for (size_t i = 0; i < kIndices.size(); ++i) {
    shuffled_guid[i] = guid[kIndices[i]];
  }

  std::string build_id;
  for (const uint8_t& byte : shuffled_guid) {
    absl::StrAppend(&build_id, absl::Hex(byte, absl::kZeroPad2));
  }

  // The dash ("-") is intentional to make it easy to distinguish the age when debugging issues
  // related to build id.
  absl::StrAppend(&build_id, absl::StrFormat("-%i", age));
  return build_id;
}

}  // namespace orbit_object_utils
