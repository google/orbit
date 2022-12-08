// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GteVector.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <memory>

#include "OrbitGl/CoreMath.h"
#include "OrbitGl/MockBatcher.h"
#include "OrbitGl/PickingManager.h"

namespace orbit_gl {

TEST(Batcher, GetBatcherId) {
  MockBatcher ui_batcher(BatcherId::kUi);
  MockBatcher timer_batcher(BatcherId::kTimeGraph);

  EXPECT_EQ(ui_batcher.GetBatcherId(), BatcherId::kUi);
  EXPECT_EQ(timer_batcher.GetBatcherId(), BatcherId::kTimeGraph);
}

TEST(Batcher, Translations) {
  MockBatcher batcher;

  const Vec3 fake_translation1{1, 2, 3};
  const uint32_t num_stack_translations = 10;

  for (uint32_t i = 0; i < num_stack_translations; i++) {
    batcher.PushTranslation(fake_translation1[0], fake_translation1[1], fake_translation1[2]);
  }
  for (uint32_t i = 0; i < num_stack_translations; i++) {
    batcher.PopTranslation();
  }

  EXPECT_DEATH(batcher.PopTranslation(), "empty");
}

}  // namespace orbit_gl