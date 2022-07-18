// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "PresentEventManager.h"

namespace orbit_gl {

using orbit_grpc_protos::PresentEvent;

TEST(PresentEventManager, OutOfOrderEventsFails) {
  PresentEventManager present_event_manager;
  constexpr PresentEvent::Source kSource = orbit_grpc_protos::PresentEvent::kDxgi;
  present_event_manager.ExchangeLastTimeStampForSource(kSource, 1);
  EXPECT_DEATH(present_event_manager.ExchangeLastTimeStampForSource(kSource, 0), "");
}

TEST(PresentEventManager, ExchangeReturnValues) {
  PresentEventManager present_event_manager;
  constexpr PresentEvent::Source kSource = orbit_grpc_protos::PresentEvent::kDxgi;
  std::optional<uint64_t> result = present_event_manager.ExchangeLastTimeStampForSource(kSource, 0);
  EXPECT_FALSE(result.has_value());

  result = present_event_manager.ExchangeLastTimeStampForSource(kSource, 1);
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 0);

  result = present_event_manager.ExchangeLastTimeStampForSource(kSource, 2);
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 1);
}

}  // namespace orbit_gl
