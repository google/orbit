// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>
#include <string.h>

#include <string>

#include "OrbitBase/Tracing.h"

static orbit_api::Event Decode(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                               uint64_t a6) {
  orbit_api::EncodedEvent encoded_event(a1, a2, a3, a4, a5, a6);
  return encoded_event.event;
}

TEST(OrbitApi, Encoding) {
  constexpr orbit_api::EventType kType = orbit_api::kTrackInt64;
  constexpr const char* kName = "The quick brown fox jumps over the lazy dog";
  constexpr double kData = 1234567.12345671234567;
  constexpr orbit_api_color kColor = kOrbitColorAmber;

  orbit_api::EncodedEvent e(kType, kName, orbit_api::Encode<uint64_t>(kData), kColor);
  auto decoded_event = Decode(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);

  EXPECT_EQ(decoded_event.type, kType);
  EXPECT_EQ(orbit_api::Decode<double>(decoded_event.data), kData);
  EXPECT_EQ(decoded_event.color, kColor);

  std::string initial_string(kName);
  EXPECT_EQ(strlen(decoded_event.name), orbit_api::kMaxEventStringSize - 1);
  EXPECT_TRUE(initial_string.find(decoded_event.name) != std::string::npos);
}
