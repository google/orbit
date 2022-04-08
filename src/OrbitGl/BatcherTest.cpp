// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "Batcher.h"
#include "MockBatcher.h"

namespace orbit_gl {

TEST(Batcher, GetBatcherId) {
  MockBatcher ui_batcher(BatcherId::kUi);
  MockBatcher timer_batcher(BatcherId::kTimeGraph);

  EXPECT_EQ(ui_batcher.GetBatcherId(), BatcherId::kUi);
  EXPECT_EQ(timer_batcher.GetBatcherId(), BatcherId::kTimeGraph);
}

TEST(Batcher, Translations) {
  MockBatcher batcher;

  const Vec3 kFakeTranslation1{1, 2, 3};
  const uint32_t kNumStackTranslations = 10;

  for (uint32_t i = 0; i < kNumStackTranslations; i++) {
    batcher.PushTranslation(kFakeTranslation1[0], kFakeTranslation1[1], kFakeTranslation1[2]);
  }
  for (uint32_t i = 0; i < kNumStackTranslations; i++) {
    batcher.PopTranslation();
  }

  EXPECT_DEATH(batcher.PopTranslation(), "empty");
}

}  // namespace orbit_gl