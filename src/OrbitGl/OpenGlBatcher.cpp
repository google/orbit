// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/OpenGlBatcher.h"

#include <GteVector.h>
#include <GteVector2.h>
#include <stddef.h>

#include <QOpenGLFunctions>
#include <algorithm>
#include <utility>

#include "Introspection/Introspection.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/BatchRenderGroup.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/TranslationStack.h"

namespace orbit_gl {

void OpenGlBatcher::ResetElements() {
  for (auto& [unused_group_id, buffer] : primitive_buffers_by_group_) {
    buffer.Reset();
  }
  user_data_.clear();
  ORBIT_CHECK(translations_.IsEmpty());
  current_render_group_ = BatchRenderGroupId();
}

static void MoveLineToPixelCenterIfHorizontal(Line& line) {
  if (line.start_point[1] != line.end_point[1]) return;
  line.start_point[1] += 0.5f;
  line.end_point[1] += 0.5f;
}

void OpenGlBatcher::AddLine(Vec2 from, Vec2 to, float z, const Color& color,
                            const Color& picking_color,
                            std::unique_ptr<PickingUserData> user_data) {
  Line line;
  LayeredVec2 translated_start_with_z =
      translations_.TranslateXYZAndFloorXY({{from[0], from[1]}, z});
  line.start_point = translated_start_with_z.xy;
  const float layer_z_value = translated_start_with_z.z;

  line.end_point = translations_.TranslateXYZAndFloorXY({{to[0], to[1]}, z}).xy;
  // TODO(b/195386885) This is a hack to address the issue that some horizontal lines in the graph
  // tracks are missing. We need a better solution for this issue.
  MoveLineToPixelCenterIfHorizontal(line);
  current_render_group_.layer = layer_z_value;
  auto& buffer = primitive_buffers_by_group_[current_render_group_];

  buffer.line_buffer.lines_.emplace_back(line);
  buffer.line_buffer.colors_.push_back_n(color, 2);
  buffer.line_buffer.picking_colors_.push_back_n(picking_color, 2);
  user_data_.push_back(std::move(user_data));
}

void OpenGlBatcher::AddBox(const Quad& box, float z, const std::array<Color, 4>& colors,
                           const Color& picking_color, std::unique_ptr<PickingUserData> user_data) {
  Quad rounded_box = box;
  float layer_z_value{};
  for (size_t v = 0; v < 4; ++v) {
    LayeredVec2 layered_vec2 = translations_.TranslateXYZAndFloorXY({rounded_box.vertices[v], z});
    rounded_box.vertices[v] = layered_vec2.xy;
    layer_z_value = layered_vec2.z;
  }

  current_render_group_.layer = layer_z_value;
  auto& buffer = primitive_buffers_by_group_[current_render_group_];
  buffer.box_buffer.boxes_.emplace_back(rounded_box);
  buffer.box_buffer.colors_.push_back(colors);
  buffer.box_buffer.picking_colors_.push_back_n(picking_color, 4);
  user_data_.push_back(std::move(user_data));
}

void OpenGlBatcher::AddTriangle(const Triangle& triangle, float z,
                                const std::array<Color, 3>& colors, const Color& picking_color,
                                std::unique_ptr<PickingUserData> user_data) {
  Triangle rounded_tri = triangle;
  float layer_z_value{};
  for (auto& vertex : rounded_tri.vertices) {
    LayeredVec2 layered_vec2 = translations_.TranslateXYZAndFloorXY({vertex, z});
    vertex = layered_vec2.xy;
    layer_z_value = layered_vec2.z;
  }

  current_render_group_.layer = layer_z_value;
  auto& buffer = primitive_buffers_by_group_[current_render_group_];
  buffer.triangle_buffer.triangles_.emplace_back(rounded_tri);
  buffer.triangle_buffer.colors_.push_back(colors);
  buffer.triangle_buffer.picking_colors_.push_back_n(picking_color, 3);
  user_data_.push_back(std::move(user_data));
}

[[nodiscard]] std::vector<BatchRenderGroupId> OpenGlBatcher::GetNonEmptyRenderGroups() const {
  std::vector<BatchRenderGroupId> result;
  for (const auto& [group, buffers] : primitive_buffers_by_group_) {
    if (buffers.box_buffer.boxes_.size() == 0 && buffers.line_buffer.lines_.size() == 0 &&
        buffers.triangle_buffer.triangles_.size() == 0) {
      continue;
    }
    result.push_back(group);
  }
  return result;
};

void OpenGlBatcher::DrawRenderGroup(const BatchRenderGroupId& group, bool picking) {
  ORBIT_SCOPE_FUNCTION;
  if (primitive_buffers_by_group_.count(group) == 0u) return;
  initializeOpenGLFunctions();
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  if (picking) {
    glDisable(GL_BLEND);
  } else {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glEnable(GL_TEXTURE_2D);

  DrawBoxBuffer(group, picking);
  DrawLineBuffer(group, picking);
  DrawTriangleBuffer(group, picking);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glPopAttrib();
  glPopClientAttrib();
}

void OpenGlBatcher::DrawBoxBuffer(const BatchRenderGroupId& group, bool picking) {
  auto& box_buffer = primitive_buffers_by_group_.at(group).box_buffer;
  const orbit_containers::Block<Quad, orbit_gl_internal::BoxBuffer::NUM_BOXES_PER_BLOCK>*
      box_block = box_buffer.boxes_.root();
  const orbit_containers::Block<Color, orbit_gl_internal::BoxBuffer::NUM_BOXES_PER_BLOCK* 4>*
      color_block = !picking ? box_buffer.colors_.root() : box_buffer.picking_colors_.root();

  while (box_block != nullptr) {
    if (auto num_elems = box_block->size()) {
      glVertexPointer(2, GL_FLOAT, sizeof(Vec2), box_block->data());
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color), color_block->data());
      glDrawArrays(GL_QUADS, 0, num_elems * 4);
    }

    box_block = box_block->next();
    color_block = color_block->next();
  }
}

void OpenGlBatcher::DrawLineBuffer(const BatchRenderGroupId& group, bool picking) {
  auto& line_buffer = primitive_buffers_by_group_.at(group).line_buffer;
  const orbit_containers::Block<Line, orbit_gl_internal::LineBuffer::NUM_LINES_PER_BLOCK>*
      line_block = line_buffer.lines_.root();
  const orbit_containers::Block<Color, orbit_gl_internal::LineBuffer::NUM_LINES_PER_BLOCK* 2>*
      color_block = !picking ? line_buffer.colors_.root() : line_buffer.picking_colors_.root();
  while (line_block != nullptr) {
    if (auto num_elems = line_block->size()) {
      glVertexPointer(2, GL_FLOAT, sizeof(Vec2), line_block->data());
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color), color_block->data());
      glDrawArrays(GL_LINES, 0, num_elems * 2);
    }

    line_block = line_block->next();
    color_block = color_block->next();
  }
}

void OpenGlBatcher::DrawTriangleBuffer(const BatchRenderGroupId& group, bool picking) {
  auto& triangle_buffer = primitive_buffers_by_group_.at(group).triangle_buffer;
  const orbit_containers::Block<
      Triangle, orbit_gl_internal::TriangleBuffer::NUM_TRIANGLES_PER_BLOCK>* triangle_block =
      triangle_buffer.triangles_.root();
  const orbit_containers::Block<
      Color, orbit_gl_internal::TriangleBuffer::NUM_TRIANGLES_PER_BLOCK* 3>* color_block =
      !picking ? triangle_buffer.colors_.root() : triangle_buffer.picking_colors_.root();

  while (triangle_block != nullptr) {
    if (int num_elems = triangle_block->size()) {
      glVertexPointer(2, GL_FLOAT, sizeof(Vec2), triangle_block->data());
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color), color_block->data());
      glDrawArrays(GL_TRIANGLES, 0, num_elems * 3);
    }

    triangle_block = triangle_block->next();
    color_block = color_block->next();
  }
}

const PickingUserData* OpenGlBatcher::GetUserData(PickingId id) const {
  ORBIT_CHECK(id.element_id >= 0);
  ORBIT_CHECK(id.batcher_id == GetBatcherId());

  switch (id.type) {
    case PickingType::kInvalid:
      return nullptr;
    case PickingType::kBox:
    case PickingType::kTriangle:
    case PickingType::kLine:
      ORBIT_CHECK(id.element_id < user_data_.size());
      return user_data_[id.element_id].get();
    case PickingType::kPickable:
      return nullptr;
    case PickingType::kCount:
      ORBIT_UNREACHABLE();
  }

  ORBIT_UNREACHABLE();
}

namespace {
template <class T, uint32_t size>
size_t CalculateBlockChainMemory(const orbit_containers::BlockChain<T, size>& chain) {
  size_t result = 0;

  auto block = chain.root();
  while (block != nullptr) {
    result += size * sizeof(T);
    block = block->next();
  }

  return result;
}

template <class T, uint32_t size>
size_t CalculateBlockChainNonEmptyBlockCount(const orbit_containers::BlockChain<T, size>& chain) {
  size_t result = 0;

  auto block = chain.root();
  while (block != nullptr) {
    if (block->size() != 0) result++;
    block = block->next();
  }

  return result;
}
}  // namespace

[[nodiscard]] Batcher::Statistics OpenGlBatcher::GetStatistics() const {
  Statistics result;

  for (const auto& layer : primitive_buffers_by_group_) {
    result.reserved_memory += CalculateBlockChainMemory(layer.second.box_buffer.boxes_);
    result.reserved_memory += CalculateBlockChainMemory(layer.second.box_buffer.picking_colors_);
    result.reserved_memory += CalculateBlockChainMemory(layer.second.box_buffer.colors_);
    result.reserved_memory += CalculateBlockChainMemory(layer.second.line_buffer.lines_);
    result.reserved_memory += CalculateBlockChainMemory(layer.second.line_buffer.picking_colors_);
    result.reserved_memory += CalculateBlockChainMemory(layer.second.line_buffer.colors_);
    result.reserved_memory += CalculateBlockChainMemory(layer.second.triangle_buffer.triangles_);
    result.reserved_memory +=
        CalculateBlockChainMemory(layer.second.triangle_buffer.picking_colors_);
    result.reserved_memory += CalculateBlockChainMemory(layer.second.triangle_buffer.colors_);

    result.stored_vertices += layer.second.box_buffer.boxes_.size() * 4;
    result.stored_vertices += layer.second.line_buffer.lines_.size() * 2;
    result.stored_vertices += layer.second.triangle_buffer.triangles_.size() * 3;

    result.draw_calls += CalculateBlockChainNonEmptyBlockCount(layer.second.box_buffer.boxes_);
    result.draw_calls += CalculateBlockChainNonEmptyBlockCount(layer.second.line_buffer.lines_);
    result.draw_calls +=
        CalculateBlockChainNonEmptyBlockCount(layer.second.triangle_buffer.triangles_);
  }

  result.stored_layers = primitive_buffers_by_group_.size();

  return result;
}

}  // namespace orbit_gl
