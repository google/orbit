// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GteVector.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Containers/BlockChain.h"
#include "OrbitGl/BatcherInterface.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/OpenGlBatcher.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/PickingManagerTest.h"

namespace orbit_gl {

class FakeOpenGlBatcher : public OpenGlBatcher {
 public:
  explicit FakeOpenGlBatcher(BatcherId id) : OpenGlBatcher(id) {}

  void ResetMockDrawCounts() {
    drawn_line_colors_.clear();
    drawn_triangle_colors_.clear();
    drawn_box_colors_.clear();
  }

  // Auxiliary methods to simplify the addition of lines, boxes and triangles.
  void AddLineHelper(const Vec2& from, const Vec2& to, float z, const Color& color,
                     std::unique_ptr<PickingUserData> user_data = nullptr) {
    Color picking_color = PickingId::ToColor(PickingType::kLine, GetNumElements(), GetBatcherId());
    return AddLine(from, to, z, color, picking_color, std::move(user_data));
  }
  void AddBoxHelper(const Quad& box, float z, const Color& color,
                    std::unique_ptr<PickingUserData> user_data = nullptr) {
    Color picking_color = PickingId::ToColor(PickingType::kBox, GetNumElements(), GetBatcherId());
    return AddBox(box, z, {color, color, color, color}, picking_color, std::move(user_data));
  }
  void AddTriangleHelper(const Triangle& triangle, float z, const Color& color,
                         std::unique_ptr<PickingUserData> user_data = nullptr) {
    Color picking_color =
        PickingId::ToColor(PickingType::kTriangle, GetNumElements(), GetBatcherId());
    return AddTriangle(triangle, z, {color, color, color}, picking_color, std::move(user_data));
  }
  const std::vector<Color>& GetDrawnLineColors() const { return drawn_line_colors_; }
  const std::vector<Color>& GetDrawnTriangleColors() const { return drawn_triangle_colors_; }
  const std::vector<Color>& GetDrawnBoxColors() const { return drawn_box_colors_; }

  // Simulate drawing by simple appending all colors to internal
  // buffers. Only a single color per element will be appended
  // (start point for line, first vertex for triangle and box)
  void DrawLayer(float layer, bool picking = false) override {
    auto& buffer = primitive_buffers_by_layer_.at(layer);
    if (picking) {
      for (auto it = buffer.line_buffer.picking_colors_.begin();
           it != buffer.line_buffer.picking_colors_.end();) {
        drawn_line_colors_.push_back(*it);
        ++it;
        ++it;
      }
      for (auto it = buffer.triangle_buffer.picking_colors_.begin();
           it != buffer.triangle_buffer.picking_colors_.end();) {
        drawn_triangle_colors_.push_back(*it);
        ++it;
        ++it;
        ++it;
      }
      for (auto it = buffer.box_buffer.picking_colors_.begin();
           it != buffer.box_buffer.picking_colors_.end();) {
        drawn_box_colors_.push_back(*it);
        ++it;
        ++it;
        ++it;
        ++it;
      }
    } else {
      for (auto it = buffer.line_buffer.colors_.begin(); it != buffer.line_buffer.colors_.end();) {
        drawn_line_colors_.push_back(*it);
        ++it;
        ++it;
      }
      for (auto it = buffer.triangle_buffer.colors_.begin();
           it != buffer.triangle_buffer.colors_.end();) {
        drawn_triangle_colors_.push_back(*it);
        ++it;
        ++it;
        ++it;
      }
      for (auto it = buffer.box_buffer.colors_.begin(); it != buffer.box_buffer.colors_.end();) {
        drawn_box_colors_.push_back(*it);
        ++it;
        ++it;
        ++it;
        ++it;
      }
    }
  }

  const orbit_gl_internal::PrimitiveBuffers& GetInternalBuffers(float layer) const {
    return primitive_buffers_by_layer_.at(layer);
  }

 private:
  mutable std::vector<Color> drawn_line_colors_;
  mutable std::vector<Color> drawn_triangle_colors_;
  mutable std::vector<Color> drawn_box_colors_;
};

void ExpectDraw(FakeOpenGlBatcher& batcher, uint32_t line_count, uint32_t triangle_count,
                uint32_t box_count) {
  batcher.ResetMockDrawCounts();

  for (auto layer : batcher.GetLayers()) {
    batcher.DrawLayer(layer);
  }
  EXPECT_EQ(batcher.GetDrawnLineColors().size(), line_count);
  EXPECT_EQ(batcher.GetDrawnTriangleColors().size(), triangle_count);
  EXPECT_EQ(batcher.GetDrawnBoxColors().size(), box_count);
}

TEST(OpenGlBatcher, SimpleElementsDrawing) {
  FakeOpenGlBatcher batcher(BatcherId::kUi);

  ExpectDraw(batcher, 0, 0, 0);
  batcher.AddLineHelper(Vec2(0, 0), Vec2(1, 0), 0, Color(255, 255, 255, 255));
  ExpectDraw(batcher, 1, 0, 0);
  EXPECT_EQ(batcher.GetDrawnLineColors()[0], Color(255, 255, 255, 255));
  batcher.AddTriangleHelper(Triangle(Vec2(0, 0), Vec2(0, 1), Vec2(1, 0)), 0, Color(0, 255, 0, 255));
  ExpectDraw(batcher, 1, 1, 0);
  EXPECT_EQ(batcher.GetDrawnTriangleColors()[0], Color(0, 255, 0, 255));
  batcher.AddBoxHelper(MakeBox(Vec2(0, 0), Vec2(1, 1)), 0, Color(255, 0, 0, 255));
  ExpectDraw(batcher, 1, 1, 1);
  EXPECT_EQ(batcher.GetDrawnBoxColors()[0], Color(255, 0, 0, 255));
  batcher.ResetElements();
  ExpectDraw(batcher, 0, 0, 0);
}

template <typename T>
void ExpectCustomDataEq(const FakeOpenGlBatcher& batcher, const Color& rendered_color,
                        const T& value) {
  PickingId id = MockRenderPickingColor(rendered_color);
  const PickingUserData* rendered_data = batcher.GetUserData(id);
  EXPECT_NE(rendered_data, nullptr);
  EXPECT_NE(rendered_data->custom_data_, nullptr);
  EXPECT_EQ(*static_cast<const T*>(rendered_data->custom_data_), value);
}

TEST(OpenGlBatcher, PickingSimpleElements) {
  FakeOpenGlBatcher batcher(BatcherId::kUi);
  EXPECT_EQ(batcher.GetBatcherId(), BatcherId::kUi);

  std::string line_custom_data = "line custom data";
  auto line_user_data = std::make_unique<PickingUserData>();
  line_user_data->custom_data_ = &line_custom_data;

  std::string triangle_custom_data = "triangle custom data";
  auto triangle_user_data = std::make_unique<PickingUserData>();
  triangle_user_data->custom_data_ = &triangle_custom_data;

  std::string box_custom_data = "box custom data";
  auto box_user_data = std::make_unique<PickingUserData>();
  box_user_data->custom_data_ = &box_custom_data;

  batcher.AddLineHelper(Vec2(0, 0), Vec2(1, 0), 0, Color(255, 255, 255, 255),
                        std::move(line_user_data));
  batcher.AddTriangleHelper(Triangle(Vec2(0, 0), Vec2(0, 1), Vec2(1, 0)), 0, Color(0, 255, 0, 255),
                            std::move(triangle_user_data));
  batcher.AddBoxHelper(MakeBox(Vec2(0, 0), Vec2(1, 1)), 0, Color(255, 0, 0, 255),
                       std::move(box_user_data));

  for (auto layer : batcher.GetLayers()) {
    batcher.DrawLayer(layer, true);
  }

  ExpectCustomDataEq(batcher, batcher.GetDrawnLineColors()[0], line_custom_data);
  ExpectCustomDataEq(batcher, batcher.GetDrawnTriangleColors()[0], triangle_custom_data);
  ExpectCustomDataEq(batcher, batcher.GetDrawnBoxColors()[0], box_custom_data);
}

TEST(OpenGlBatcher, MultipleDrawCalls) {
  FakeOpenGlBatcher batcher(BatcherId::kUi);

  std::string line_custom_data = "line custom data";
  auto line_user_data = std::make_unique<PickingUserData>();
  line_user_data->custom_data_ = &line_custom_data;

  std::string triangle_custom_data = "triangle custom data";
  auto triangle_user_data = std::make_unique<PickingUserData>();
  triangle_user_data->custom_data_ = &triangle_custom_data;

  std::string box_custom_data = "box custom data";
  auto box_user_data = std::make_unique<PickingUserData>();
  box_user_data->custom_data_ = &box_custom_data;

  batcher.AddLineHelper(Vec2(0, 0), Vec2(1, 0), 0, Color(255, 255, 255, 255),
                        std::move(line_user_data));
  batcher.AddTriangleHelper(Triangle(Vec2(0, 0), Vec2(0, 1), Vec2(1, 0)), 0, Color(0, 255, 0, 255),
                            std::move(triangle_user_data));
  batcher.AddBoxHelper(MakeBox(Vec2(0, 0), Vec2(1, 1)), 0, Color(255, 0, 0, 255),
                       std::move(box_user_data));

  for (auto layer : batcher.GetLayers()) {
    batcher.DrawLayer(layer, true);
  }

  auto line_color = batcher.GetDrawnLineColors()[0];
  auto triangle_color = batcher.GetDrawnTriangleColors()[0];
  auto box_color = batcher.GetDrawnBoxColors()[0];
  ExpectCustomDataEq(batcher, line_color, line_custom_data);
  ExpectCustomDataEq(batcher, triangle_color, triangle_custom_data);
  ExpectCustomDataEq(batcher, box_color, box_custom_data);

  batcher.ResetElements();
  PickingId id = MockRenderPickingColor(line_color);
  EXPECT_DEATH((void)batcher.GetUserData(id), "size");
  id = MockRenderPickingColor(triangle_color);
  EXPECT_DEATH((void)batcher.GetUserData(id), "size");
  id = MockRenderPickingColor(box_color);
  EXPECT_DEATH((void)batcher.GetUserData(id), "size");
}

struct Line3D {
  Vec3 start_point;
  Vec3 end_point;
};

void LineEq(const Line3D& lhs, const Line& rhs) {
  EXPECT_EQ(lhs.start_point[0], rhs.start_point[0]);
  EXPECT_EQ(lhs.start_point[1], rhs.start_point[1]);

  EXPECT_EQ(lhs.end_point[0], rhs.end_point[0]);
  EXPECT_EQ(lhs.end_point[1], rhs.end_point[1]);
}

TEST(OpenGlBatcher, TranslationsAreAutomaticallyAdded) {
  FakeOpenGlBatcher batcher(BatcherId::kUi);

  batcher.AddLineHelper(Vec2(0.f, 0.f), Vec2(1.f, 1.f), 0.f, Color());

  const Line3D original_expectation{Vec3(0.f, 0.f, 0.f), Vec3(1.f, 1.f, 0.f)};
  const Vec3 transform(10.f, 100.f, 0.1f);
  const Line3D transformed_expectation{original_expectation.start_point + transform,
                                       original_expectation.end_point + transform};

  const auto first_from_layer = [&batcher = std::as_const(batcher)](float z) {
    return *batcher.GetInternalBuffers(z).line_buffer.lines_.begin();
  };

  const auto add_line_assert_eq = [&batcher, &first_from_layer](const Line3D& expectation) {
    batcher.AddLineHelper(Vec2(0.f, 0.f), Vec2(1.f, 1.f), 0.f, Color());
    float expected_z = expectation.end_point[2];
    LineEq(expectation, first_from_layer(expected_z));
  };

  LineEq(original_expectation, first_from_layer(0.f));

  batcher.PushTranslation(10, 100, 0.1f);
  // Should not affect previously added lines
  LineEq(original_expectation, first_from_layer(0.f));

  add_line_assert_eq(transformed_expectation);
  batcher.PushTranslation(0, 0, 0.f);
  add_line_assert_eq(transformed_expectation);
  batcher.PopTranslation();
  add_line_assert_eq(transformed_expectation);
  batcher.PopTranslation();
  add_line_assert_eq(original_expectation);
  ASSERT_DEATH(batcher.PopTranslation(), "Check failed");
}

}  // namespace orbit_gl
