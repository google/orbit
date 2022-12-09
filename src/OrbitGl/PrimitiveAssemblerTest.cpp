// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GteVector.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <algorithm>
#include <array>
#include <memory>

#include "OrbitGl/BatcherInterface.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/MockBatcher.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/PickingManagerTest.h"
#include "OrbitGl/PrimitiveAssembler.h"

namespace orbit_gl {

namespace {

const Color kFakeColor{42, 42, 128, 43};
const Vec2 kTopLeft{0, 0};
const Vec2 kTopRight{5, 0};
const Vec2 kBottomRight{5, 5};
const Vec2 kBottomLeft{0, 5};
const Vec2 kBoxSize{5, 5};
const Vec2 kArrowStartingPos{5, 5};
const Vec2 kArrowBodySize{2, 4};
const Vec2 kArrowHeadSize{4, 2};

class PrimitiveAssemblerTester : public PrimitiveAssembler {
 public:
  explicit PrimitiveAssemblerTester(PickingManager* picking_manager = nullptr)
      : PrimitiveAssembler(&mock_batcher_, picking_manager) {}
  [[nodiscard]] uint32_t GetNumLines() const { return mock_batcher_.GetNumLines(); }
  [[nodiscard]] uint32_t GetNumTriangles() const { return mock_batcher_.GetNumTriangles(); }
  [[nodiscard]] uint32_t GetNumBoxes() const { return mock_batcher_.GetNumBoxes(); }
  [[nodiscard]] uint32_t GetNumElements() const { return mock_batcher_.GetNumElements(); }
  [[nodiscard]] bool IsEverythingInsideRectangle(const Vec2& pos, const Vec2& size) const {
    return mock_batcher_.IsEverythingInsideRectangle(pos, size);
  }

 private:
  MockBatcher mock_batcher_;
};

}  // namespace

// TODO(http://b/228063067): Test all methods in Primitive Assembler
TEST(PrimitiveAssembler, PickableInNullPickingManager) {
  PrimitiveAssemblerTester primitive_assembler_tester;
  std::shared_ptr<PickableMock> pickable = std::make_shared<PickableMock>();
  EXPECT_DEATH(primitive_assembler_tester.AddLine(Vec2(0, 0), Vec2(1, 0), 0,
                                                  Color(255, 255, 255, 255), pickable),
               "nullptr");
}

TEST(PrimitiveAssembler, StartNewFrame) {
  PrimitiveAssemblerTester primitive_assembler_tester;
  primitive_assembler_tester.AddLine(kTopLeft, kTopRight, 0, kFakeColor);
  EXPECT_EQ(primitive_assembler_tester.GetNumLines(), 1);

  primitive_assembler_tester.StartNewFrame();
  EXPECT_EQ(primitive_assembler_tester.GetNumLines(), 0);
}

TEST(PrimitiveAssembler, BasicAdditions) {
  PickingManager pm;
  PrimitiveAssemblerTester primitive_assembler_tester(&pm);
  std::shared_ptr<PickableMock> pickable = std::make_shared<PickableMock>();

  constexpr uint32_t kNumLines = 4;
  constexpr uint32_t kNumTriangles = 2;
  constexpr uint32_t kNumBoxes = 3;

  // Lines
  constexpr float kLineSize = 5.;
  primitive_assembler_tester.AddLine(kTopLeft, kBottomLeft, 0, kFakeColor);
  primitive_assembler_tester.AddLine(kTopLeft, kBottomLeft, 0, kFakeColor, pickable);
  primitive_assembler_tester.AddVerticalLine(kTopLeft, kLineSize, 0, kFakeColor);
  primitive_assembler_tester.AddVerticalLine(kTopLeft, kLineSize, 0, kFakeColor, pickable);
  EXPECT_EQ(primitive_assembler_tester.GetNumLines(), kNumLines);

  // Triangles
  const Triangle fake_triangle{kTopLeft, kTopRight, kBottomRight};
  primitive_assembler_tester.AddTriangle(fake_triangle, 0, kFakeColor);
  primitive_assembler_tester.AddTriangle(fake_triangle, 0, kFakeColor, pickable);
  EXPECT_EQ(primitive_assembler_tester.GetNumTriangles(), kNumTriangles);

  // Boxes
  const Quad fake_box{std::array<Vec2, 4>{kTopLeft, kTopRight, kBottomRight, kBottomLeft}};
  primitive_assembler_tester.AddBox(fake_box, 0, {kFakeColor, kFakeColor, kFakeColor, kFakeColor});
  primitive_assembler_tester.AddBox(fake_box, 0, kFakeColor);
  primitive_assembler_tester.AddBox(fake_box, 0, kFakeColor, pickable);
  EXPECT_EQ(primitive_assembler_tester.GetNumBoxes(), kNumBoxes);
  EXPECT_EQ(primitive_assembler_tester.GetNumElements(), kNumLines + kNumTriangles + kNumBoxes);
  EXPECT_TRUE(primitive_assembler_tester.IsEverythingInsideRectangle(kTopLeft, kBoxSize));
}

TEST(PrimitiveAssembler, ComplexShapes) {
  PickingManager pm;
  PrimitiveAssemblerTester primitive_assembler_tester(&pm);
  std::shared_ptr<PickableMock> pickable = std::make_shared<PickableMock>();

  // AddShadedBox -> Should be just a box with shaded color.
  primitive_assembler_tester.AddShadedBox(kTopLeft, kBoxSize, 0, kFakeColor);
  primitive_assembler_tester.AddShadedBox(kTopLeft, kBoxSize, 0, kFakeColor,
                                          ShadingDirection::kRightToLeft);
  primitive_assembler_tester.AddShadedBox(kTopLeft, kBoxSize, 0, kFakeColor,
                                          std::make_unique<PickingUserData>(),
                                          ShadingDirection::kTopToBottom);
  primitive_assembler_tester.AddShadedBox(kTopLeft, kBoxSize, 0, kFakeColor, pickable,
                                          ShadingDirection::kLeftToRight);
  EXPECT_EQ(primitive_assembler_tester.GetNumBoxes(), 4);
  EXPECT_EQ(primitive_assembler_tester.GetNumElements(), 4);
  EXPECT_TRUE(primitive_assembler_tester.IsEverythingInsideRectangle(kTopLeft, kBoxSize));

  primitive_assembler_tester.StartNewFrame();

  constexpr float kRoundedRadius = 2.f;

  // AddRoundedBox -> 3 boxes + 4 rounding corners
  primitive_assembler_tester.AddRoundedBox(kTopLeft, kBoxSize, 0, kRoundedRadius, kFakeColor);
  EXPECT_EQ(primitive_assembler_tester.GetNumBoxes(), 3);
  EXPECT_EQ(primitive_assembler_tester.GetNumTriangles(),
            4 * primitive_assembler_tester.kNumArcSides);
  EXPECT_EQ(primitive_assembler_tester.GetNumLines(), 0);
  EXPECT_TRUE(primitive_assembler_tester.IsEverythingInsideRectangle(kTopLeft, kBoxSize));

  primitive_assembler_tester.StartNewFrame();

  // TODO(b/227744958) This should probably be removed and AddBox should be used instead
  // AddShadedTrapezium -> 2 Triangles
  const Vec2 top_centred = {(kTopLeft[0] + kTopRight[0]) / 2.f, kTopLeft[1]};
  primitive_assembler_tester.AddShadedTrapezium(
      Quad{{kTopLeft, top_centred, kBottomRight, kBottomLeft}}, 0, kFakeColor,
      std::make_unique<PickingUserData>());
  EXPECT_EQ(primitive_assembler_tester.GetNumTriangles(), 2);
  EXPECT_EQ(primitive_assembler_tester.GetNumElements(), 2);
  EXPECT_TRUE(primitive_assembler_tester.IsEverythingInsideRectangle(kTopLeft, kBoxSize));

  primitive_assembler_tester.StartNewFrame();

  // AddQuadBorder -> 4 Lines
  primitive_assembler_tester.AddQuadBorder(Quad{{kBottomRight, kBottomLeft, kTopLeft, kTopRight}},
                                           0, kFakeColor, std::make_unique<PickingUserData>());
  EXPECT_EQ(primitive_assembler_tester.GetNumLines(), 4);
  EXPECT_EQ(primitive_assembler_tester.GetNumElements(), 4);
  EXPECT_TRUE(primitive_assembler_tester.IsEverythingInsideRectangle(kTopLeft, kBoxSize));

  primitive_assembler_tester.StartNewFrame();

  // AddCircle -> several Triangles
  primitive_assembler_tester.AddCircle(kBottomRight, kRoundedRadius, 0, kFakeColor);
  EXPECT_EQ(primitive_assembler_tester.GetNumTriangles(), PrimitiveAssembler::kCirclePoints);
  EXPECT_EQ(primitive_assembler_tester.GetNumLines(), 0);
  EXPECT_EQ(primitive_assembler_tester.GetNumBoxes(), 0);
  EXPECT_TRUE(primitive_assembler_tester.IsEverythingInsideRectangle(
      kBottomRight - Vec2{kRoundedRadius, kRoundedRadius},
      {kRoundedRadius * 2.f, kRoundedRadius * 2.f}));

  primitive_assembler_tester.StartNewFrame();

  // AddVerticalArrow (arrow pointing down) -> 1 box and 1 Triangle
  primitive_assembler_tester.AddVerticalArrow(kArrowStartingPos, kArrowBodySize, kArrowHeadSize, 0,
                                              kFakeColor,
                                              PrimitiveAssembler::ArrowDirection::kDown);
  float arrow_width1 = std::max(kArrowHeadSize[0], kArrowBodySize[0]);
  float arrow_height1 = kArrowHeadSize[1] + kArrowBodySize[1];
  EXPECT_EQ(primitive_assembler_tester.GetNumTriangles(), 1);
  EXPECT_EQ(primitive_assembler_tester.GetNumLines(), 0);
  EXPECT_EQ(primitive_assembler_tester.GetNumBoxes(), 1);
  EXPECT_TRUE(primitive_assembler_tester.IsEverythingInsideRectangle(
      kArrowStartingPos - Vec2{arrow_width1 / 2, 0}, Vec2{arrow_width1, arrow_height1}));

  primitive_assembler_tester.StartNewFrame();

  // AddVerticalArrow (arrow pointing up) -> 1 box and 1 Triangle
  primitive_assembler_tester.AddVerticalArrow(kArrowStartingPos, kArrowBodySize, kArrowHeadSize, 0,
                                              kFakeColor, PrimitiveAssembler::ArrowDirection::kUp);
  float arrow_width2 = std::max(kArrowHeadSize[0], kArrowBodySize[0]);
  float arrow_height2 = kArrowHeadSize[1] + kArrowBodySize[1];
  EXPECT_EQ(primitive_assembler_tester.GetNumTriangles(), 1);
  EXPECT_EQ(primitive_assembler_tester.GetNumLines(), 0);
  EXPECT_EQ(primitive_assembler_tester.GetNumBoxes(), 1);
  EXPECT_TRUE(primitive_assembler_tester.IsEverythingInsideRectangle(
      kArrowStartingPos - Vec2{arrow_width2 / 2, arrow_height2},
      Vec2{arrow_width2, arrow_height2}));
}

}  // namespace orbit_gl