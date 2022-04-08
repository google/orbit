// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "MockBatcher.h"
#include "PickingManagerTest.h"
#include "PrimitiveAssembler.h"

namespace orbit_gl {

namespace {

class PrimitiveAssemblerTester : public PrimitiveAssembler {
 public:
  explicit PrimitiveAssemblerTester(PickingManager* picking_manager = nullptr)
      : PrimitiveAssembler(&mock_batcher_, picking_manager) {}
  [[nodiscard]] uint32_t GetNumLines() const { return mock_batcher_.GetNumLines(); }
  [[nodiscard]] uint32_t GetNumTriangles() const { return mock_batcher_.GetNumTriangles(); }
  [[nodiscard]] uint32_t GetNumBoxes() const { return mock_batcher_.GetNumBoxes(); }
  [[nodiscard]] uint32_t GetNumElements() const { return mock_batcher_.GetNumElements(); }

 private:
  MockBatcher mock_batcher_;
};

}  // namespace

// TODO(http://b/228063067): Test all methods in Primitive Assembler
TEST(PrimitiveAssembler, NullPickingManager) {
  PrimitiveAssemblerTester primitive_assembler_tester;
  std::shared_ptr<PickableMock> pickable = std::make_shared<PickableMock>();
  EXPECT_DEATH(primitive_assembler_tester.AddLine(Vec2(0, 0), Vec2(1, 0), 0,
                                                  Color(255, 255, 255, 255), pickable),
               "nullptr");
}

TEST(PrimitiveAssembler, BasicAdditions) {
  PickingManager pm;
  PrimitiveAssemblerTester primitive_assembler_tester(&pm);
  std::shared_ptr<PickableMock> pickable = std::make_shared<PickableMock>();

  Color kFakeColor{42, 42, 128, 43};
  Vec2 kTopLeft{0, 0};
  Vec2 kTopRight{5, 0};
  Vec2 kBottomRight{5, 5};
  Vec2 kBottomLeft{0, 5};

  constexpr uint32_t kNumLines = 4;
  constexpr uint32_t kNumTriangles = 2;
  constexpr uint32_t kNumBoxes = 3;

  // Lines
  float kLineSize = 5.;
  primitive_assembler_tester.AddLine(kTopLeft, kBottomLeft, 0, kFakeColor);
  primitive_assembler_tester.AddLine(kTopLeft, kBottomLeft, 0, kFakeColor, pickable);
  primitive_assembler_tester.AddVerticalLine(kTopLeft, kLineSize, 0, kFakeColor);
  primitive_assembler_tester.AddVerticalLine(kTopLeft, kLineSize, 0, kFakeColor, pickable);
  EXPECT_EQ(primitive_assembler_tester.GetNumLines(), kNumLines);

  // Triangles
  Triangle kFakeTriangle{kTopLeft, kTopRight, kBottomRight};
  primitive_assembler_tester.AddTriangle(kFakeTriangle, 0, kFakeColor);
  primitive_assembler_tester.AddTriangle(kFakeTriangle, 0, kFakeColor, pickable);
  EXPECT_EQ(primitive_assembler_tester.GetNumTriangles(), kNumTriangles);

  // Boxes
  Tetragon kFakeBox{std::array<Vec2, 4>{kTopLeft, kTopRight, kBottomRight, kBottomLeft}};
  primitive_assembler_tester.AddBox(kFakeBox, 0, {kFakeColor, kFakeColor, kFakeColor, kFakeColor});
  primitive_assembler_tester.AddBox(kFakeBox, 0, kFakeColor);
  primitive_assembler_tester.AddBox(kFakeBox, 0, kFakeColor, pickable);
  EXPECT_EQ(primitive_assembler_tester.GetNumBoxes(), kNumBoxes);

  EXPECT_EQ(primitive_assembler_tester.GetNumElements(), kNumLines + kNumTriangles + kNumBoxes);
}

}  // namespace orbit_gl