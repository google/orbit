// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <random>

#include "GlCanvas.h"
#include "MockBatcher.h"
#include "MockTextRenderer.h"
#include "MockTimelineInfo.h"
#include "TimelineUi.h"
#include "Viewport.h"

namespace orbit_gl {

class TimelineUiTest : public TimelineUi {
 public:
  static constexpr int kBigScreenPixelsWidth = 2000;
  static constexpr int kSmallScreenMaxPixelsWidth = 700;
  static constexpr int kTinyScreenMaxPixelsWidth = 200;

  explicit TimelineUiTest(MockTimelineInfo* mock_timeline_info, Viewport* viewport,
                          TimeGraphLayout* layout)
      : TimelineUi(nullptr /*parent*/, mock_timeline_info, viewport, layout),
        primitive_assembler_(&mock_batcher_),
        viewport_(viewport),
        mock_timeline_info_(mock_timeline_info) {
    viewport->SetWorldSize(viewport->GetWorldWidth(), TimelineUi::GetHeight());
    SetWidth(viewport_->GetWorldWidth());
  }

  void TestUpdatePrimitives(uint64_t min_tick, uint64_t max_tick) {
    mock_text_renderer_.Clear();
    mock_batcher_.ResetElements();
    mock_timeline_info_->SetMinMax(min_tick, max_tick);

    UpdatePrimitives(primitive_assembler_, mock_text_renderer_, min_tick, max_tick,
                     PickingMode::kNone);

    int num_labels = mock_text_renderer_.GetNumAddTextCalls();
    int num_boxes = mock_batcher_.GetNumBoxes();
    int num_major_ticks = mock_batcher_.GetNumLinesByColor(kTimelineMajorTickColor);
    int num_minor_ticks = mock_batcher_.GetNumLinesByColor(kTimelineMinorTickColor);

    // Lines should be only major and minor ticks.
    EXPECT_EQ(mock_batcher_.GetNumLines(), num_major_ticks + num_minor_ticks);

    // Major ticks should always be between 2 and 10, given scale set. In extremely small screens
    // (Width < 200), we don't force a minimum number of major ticks.
    if (viewport_->GetWorldWidth() > kTinyScreenMaxPixelsWidth) {
      EXPECT_GE(num_major_ticks, 2);
    }
    EXPECT_LE(num_major_ticks, 10);

    // Depending on the scale, there should be 1, 2 or 4 minor ticks between each major tick, which
    // means that (calling MT to the number of major ticks, and mt to the number of minor ticks)
    // mt is between (MT-1, MT+1), (2*(MT-1), 2*(MT+1)) or (4*(MT-1), 4*(MT+1)).
    EXPECT_GE(num_minor_ticks, num_major_ticks - 1);
    EXPECT_LE(num_minor_ticks, 4 * (num_major_ticks + 1));
    EXPECT_THAT(num_minor_ticks,
                testing::AnyOf(testing::Le(num_major_ticks + 1),
                               testing::AllOf(testing::Ge(2 * (num_major_ticks - 1)),
                                              testing::Le(2 * (num_major_ticks + 1))),
                               testing::Ge(4 * (num_major_ticks - 1))));

    // Generally, labels should all have the same number of digits, start at the same vertical
    // position and they will appear at the right of each major tick. The only exception is about
    // labels with the same number of digits when the number of hours is greater than 100. The hour
    // part of the iso timestamp will have 2 digits when it's smaller than 100.
    if (max_tick < 100 * kNanosecondsPerHour) {
      EXPECT_TRUE(mock_text_renderer_.HasAddTextsSameLength());
    }
    EXPECT_TRUE(mock_text_renderer_.AreAddTextsAlignedVertically());

    // TODO(b/218311326): In some cases, small screens (Width < 700) failed in the condition between
    // #labels and #major_ticks, since we are drawing major_ticks anyways when labels intersect.
    if (viewport_->GetWorldWidth() > kSmallScreenMaxPixelsWidth) {
      EXPECT_GE(num_labels, num_major_ticks - 1);
    }
    EXPECT_LE(num_labels, num_major_ticks + 1);

    // Boxes: One box per each label + Background box + margin box.
    EXPECT_EQ(num_boxes, num_labels + 2);

    // Everything should be between kZValueTimeBar and kZValueTimeBarLabel.
    EXPECT_TRUE(mock_text_renderer_.IsTextBetweenZLayers(GlCanvas::kZValueTimeBar,
                                                         GlCanvas::kZValueTimeBarLabel));
    EXPECT_TRUE(mock_batcher_.IsEverythingBetweenZLayers(GlCanvas::kZValueTimeBar,
                                                         GlCanvas::kZValueTimeBarLabel));

    // The label is the only thing that can be out of the expected space for the timeline.
    const char* kOneMonthLabel = "730:00:00.000000000";
    const float kMaxLabelWidth =
        mock_text_renderer_.GetStringWidth(kOneMonthLabel, layout_->GetFontSize());
    const Vec2 kExpectedMinPos{GetPos()[0] - kMaxLabelWidth, GetPos()[1]};
    const Vec2 kExpectedMaxPos{GetPos() + Vec2{GetSize()[0] + kMaxLabelWidth, GetSize()[1]}};
    EXPECT_TRUE(mock_text_renderer_.IsTextInsideRectangle(kExpectedMinPos,
                                                          kExpectedMaxPos - kExpectedMinPos));
    EXPECT_TRUE(mock_batcher_.IsEverythingInsideRectangle(kExpectedMinPos,
                                                          kExpectedMaxPos - kExpectedMinPos));
  }

  void TestDraw(uint64_t min_tick, uint64_t max_tick, std::optional<uint64_t> mouse_tick) {
    mock_text_renderer_.Clear();
    mock_batcher_.ResetElements();
    mock_timeline_info_->SetMinMax(min_tick, max_tick);

    DrawContext context;
    context.current_mouse_tick = mouse_tick;
    Draw(primitive_assembler_, mock_text_renderer_, context);

    // One box and one label, both at kZValueTimeBarMouseLabel position.
    const int kNumMouseLabels = mouse_tick.has_value() ? 1 : 0;

    EXPECT_EQ(mock_batcher_.GetNumBoxes(), kNumMouseLabels);
    EXPECT_EQ(mock_batcher_.GetNumElements(), kNumMouseLabels);
    EXPECT_EQ(mock_text_renderer_.GetNumAddTextCalls(), kNumMouseLabels);

    EXPECT_TRUE(mock_text_renderer_.IsTextBetweenZLayers(GlCanvas::kZValueTimeBarMouseLabel,
                                                         GlCanvas::kZValueTimeBarMouseLabel));
    EXPECT_TRUE(mock_batcher_.IsEverythingBetweenZLayers(GlCanvas::kZValueTimeBarMouseLabel,
                                                         GlCanvas::kZValueTimeBarMouseLabel));
  }

 private:
  MockBatcher mock_batcher_;
  MockTextRenderer mock_text_renderer_;
  PrimitiveAssembler primitive_assembler_;
  Viewport* viewport_;
  MockTimelineInfo* mock_timeline_info_;
};

static void TestUpdatePrimitivesWithSeveralRanges(int world_width) {
  MockTimelineInfo mock_timeline_info;
  mock_timeline_info.SetWorldWidth(world_width);

  TimeGraphLayout layout;
  Viewport viewport(world_width, 0);
  TimelineUiTest timeline_ui_test(&mock_timeline_info, &viewport, &layout);
  timeline_ui_test.TestUpdatePrimitives(0, 100);
  timeline_ui_test.TestUpdatePrimitives(kNanosecondsPerSecond, kNanosecondsPerSecond + 100);
  timeline_ui_test.TestUpdatePrimitives(0, kNanosecondsPerSecond - 100);
  timeline_ui_test.TestUpdatePrimitives(0, kNanosecondsPerSecond - 1);
  timeline_ui_test.TestUpdatePrimitives(0, kNanosecondsPerSecond);
  timeline_ui_test.TestUpdatePrimitives(0, 59 * kNanosecondsPerMinute);
  timeline_ui_test.TestUpdatePrimitives(90 * kNanosecondsPerHour, 100 * kNanosecondsPerHour);

  // Maximum supported timestamp: 1 Month.
  std::mt19937 gen;
  std::uniform_int_distribution<uint64_t> distrib(0, kNanosecondsPerMonth);

  for (int i = 0; i < 100; i++) {
    uint64_t timestamp_1 = distrib(gen);
    uint64_t timestamp_2 = distrib(gen);
    timeline_ui_test.TestUpdatePrimitives(std::min(timestamp_1, timestamp_2),
                                          std::max(timestamp_1, timestamp_2));
  }
}

TEST(TimelineUi, UpdatePrimitives) {
  TestUpdatePrimitivesWithSeveralRanges(TimelineUiTest::kBigScreenPixelsWidth);
  TestUpdatePrimitivesWithSeveralRanges(TimelineUiTest::kTinyScreenMaxPixelsWidth);
}

TEST(TimelineUi, Draw) {
  int width = TimelineUiTest::kBigScreenPixelsWidth;
  MockTimelineInfo mock_timeline_info;
  mock_timeline_info.SetWorldWidth(width);

  TimeGraphLayout layout;
  Viewport viewport(width, 0);
  TimelineUiTest timeline_ui_test(&mock_timeline_info, &viewport, &layout);

  const uint64_t kMinTick = 0;
  const uint64_t kMaxTick = 1000;

  // Testing different positions of the mouse in the screen. It is expected that mouse_tick is
  // between than min_tick and max_tick or either nullopt.
  timeline_ui_test.TestDraw(kMinTick, kMaxTick, /*mouse_tick=*/kMinTick);
  timeline_ui_test.TestDraw(kMinTick, kMaxTick, /*mouse_tick=*/kMinTick + 1);
  timeline_ui_test.TestDraw(kMinTick, kMaxTick, /*mouse_tick=*/(kMinTick + kMaxTick) / 2);
  timeline_ui_test.TestDraw(kMinTick, kMaxTick, /*mouse_tick=*/kMaxTick - 1);
  timeline_ui_test.TestDraw(kMinTick, kMaxTick, /*mouse_tick=*/kMaxTick);
  timeline_ui_test.TestDraw(kMinTick, kMaxTick, /*mouse_tick=*/std::nullopt);
}

}  // namespace orbit_gl