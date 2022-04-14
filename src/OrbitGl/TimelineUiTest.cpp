// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "GlCanvas.h"
#include "MockBatcher.h"
#include "MockTextRenderer.h"
#include "MockTimelineInfo.h"
#include "TimelineUi.h"
#include "Viewport.h"

namespace orbit_gl {

class TimelineUiTest : public TimelineUi {
 public:
  TimelineUiTest(int width)
      : TimelineUi(nullptr /*parent*/, &mock_timeline_info, &viewport_, &layout_),
        primitive_assembler_(&mock_batcher_),
        viewport_(width, 0) {
    mock_timeline_info.SetWorldWidth(width);
  }

  void TestUpdatePrimitives(uint64_t min_tick, uint64_t max_tick, bool is_small_screen = false) {
    mock_text_renderer_.Clear();
    mock_batcher_.ResetElements();
    mock_timeline_info.SetMinMax(min_tick, max_tick);

    UpdatePrimitives(primitive_assembler_, mock_text_renderer_, min_tick, max_tick,
                     PickingMode::kNone);

    int num_labels = mock_text_renderer_.GetNumAddTextCalls();
    int num_boxes = mock_batcher_.GetNumBoxes();
    int num_major_ticks = mock_batcher_.GetNumLinesByColor(kMajorTickColor);
    int num_minor_ticks = mock_batcher_.GetNumLinesByColor(kMinorTickColor);

    // Lines should be only major and minor ticks.
    EXPECT_EQ(mock_batcher_.GetNumLines(), num_major_ticks + num_minor_ticks);

    // Major ticks should always be between 2 and 10, given scale set. In extremely small screens
    // (Width < 160), we don't force a minimum number of major ticks.
    if (!is_small_screen) EXPECT_GE(num_major_ticks, 2);
    EXPECT_LE(num_major_ticks, 10);

    // Depending on the scale, there should be 1 minor ticks or 4 between each major tick, which
    // means that (calling MT to the number of major ticks, and mt to the number of minor ticks)
    // mt is between (MT-1, MT+1) or mt is between (4x(MT-1), 4x(MT+1)).
    EXPECT_GE(num_minor_ticks, num_major_ticks - 1);
    EXPECT_LE(num_minor_ticks, 4 * (num_major_ticks + 1));
    EXPECT_TRUE(num_minor_ticks <= num_major_ticks + 1 ||
                num_minor_ticks >= 4 * (num_major_ticks - 1));

    // Labels should all have the same number of numbers, start at the same vertical position and
    // they will appear at the right of each major tick.
    EXPECT_TRUE(mock_text_renderer_.HasAddTextsSameLength());
    EXPECT_TRUE(mock_text_renderer_.AreAddTextsAlignedVertically());
    EXPECT_GE(num_labels, num_major_ticks - 1);
    EXPECT_LE(num_labels, num_major_ticks + 1);

    // Boxes: One box per each label + Background box + margin box.
    EXPECT_EQ(num_boxes, num_labels + 2);

    // Everything should be between kZValueTimeBar and kZValueTimeBarMouseLabel.
    EXPECT_TRUE(mock_text_renderer_.IsTextBetweenZLayers(GlCanvas::kZValueTimeBar,
                                                         GlCanvas::kZValueTimeBarMouseLabel));
    EXPECT_TRUE(mock_batcher_.IsEverythingBetweenZLayers(GlCanvas::kZValueTimeBar,
                                                         GlCanvas::kZValueTimeBarMouseLabel));
  }

 private:
  MockBatcher mock_batcher_;
  MockTextRenderer mock_text_renderer_;
  PrimitiveAssembler primitive_assembler_;
  TimeGraphLayout layout_;
  Viewport viewport_;
  MockTimelineInfo mock_timeline_info;
};

TEST(TimelineUI, UpdatePrimitivesBigScreen) {
  // TODO(b/218311326): In some cases, small screens and even medium screens (Width < 700) failed in
  // the condition between #labels and #major_ticks, since we are drawing major_ticks anyways when
  // labels intersect.
  const int kBigScreenWidth = 2000;
  TimelineUiTest timeline_ui_test(kBigScreenWidth);
  timeline_ui_test.TestUpdatePrimitives(0, 100);
  for (int i = 0; i < 100; i++) {
    uint64_t timestamp_1 = rand();
    uint64_t timestamp_2 = rand();
    timeline_ui_test.TestUpdatePrimitives(std::min(timestamp_1, timestamp_2),
                                          std::max(timestamp_1, timestamp_2));
  }
}

}  // namespace orbit_gl