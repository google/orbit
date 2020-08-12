// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_PICKING_MANAGER_H
#define ORBIT_GL_PICKING_MANAGER_H

#include <cstring>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "CoreMath.h"
#include "absl/base/casts.h"
#include "absl/synchronization/mutex.h"

class GlCanvas;

enum class PickingMode { kNone, kHover, kClick };

class Pickable {
 public:
  virtual ~Pickable() = default;
  virtual void OnPick(int a_X, int a_Y) = 0;
  virtual void OnDrag(int /*a_X*/, int /*a_Y*/) {}
  virtual void OnRelease(){};
  virtual void Draw(GlCanvas* a_Canvas, PickingMode a_PickingMode) = 0;
  [[nodiscard]] virtual bool Draggable() { return false; }
  [[nodiscard]] virtual bool Movable() { return false; }
  [[nodiscard]] virtual std::string GetTooltip() const { return ""; }
};

enum class PickingType : uint32_t {
  kInvalid = 0,
  kLine = 1,
  kBox = 2,
  kTriangle = 3,
  kPickable = 4
};

// Instances of batchers used to draw must be in 1:1 correspondence with
// values in the following enum. Currently, two batchers are used, one to draw
// UI elements (corresponding to BatcherId::UI), and one for drawing events on
// the time graph (corresponding to BatcherId::kTimeGraph). If you want to add
// more batchers, this enum must be extended and you need to spend more bits
// on the batcher_id_ field below. The total number of elements that can be
// correctly picked is limited to the number of elements that can be encoded
// in the bits remaining after encoding the batcher id, and the
// PickingType, so adding more batchers or types has to be carefully
// considered.
enum class BatcherId : uint32_t { kTimeGraph, kUi };

struct PickingId {
  static constexpr const uint32_t kIDBitSize = 28;
  static constexpr const uint32_t kPickingTypeBitSize = 3;
  static constexpr const uint32_t kBatcherIDBitSize = 1;

  [[nodiscard]] inline static PickingId Get(
      PickingType type, uint32_t id,
      BatcherId batcher_id = BatcherId::kTimeGraph) {
    PickingId result;
    result.type = type;
    result.id = id;
    result.batcher_id = batcher_id;
    return result;
  }

  [[nodiscard]] static Color GetColor(
      PickingType type, uint32_t id,
      BatcherId batcher_id = BatcherId::kTimeGraph) {
    PickingId result_id = Get(type, id, batcher_id);
    std::array<uint8_t, 4> color_values;
    color_values = absl::bit_cast<std::array<uint8_t, 4>, PickingId>(result_id);
    return Color(color_values[0], color_values[1], color_values[2],
                 color_values[3]);
  }

  [[nodiscard]] inline static PickingId Get(uint32_t value) {
    PickingId id = absl::bit_cast<PickingId, uint32_t>(value);
    return id;
  }
  
  uint32_t id_ : kIDBitSize;
  PickingType type_ : kPickingTypeBitSize;
  BatcherId batcher_id_ : kBatcherIDBitSize;
};

class PickingManager {
 public:
  PickingManager() {}
  void Reset();

  void Pick(uint32_t id, int x, int y);
  void Release();
  void Drag(int x, int y);
  [[nodiscard]] std::weak_ptr<Pickable> GetPicked() const;
  [[nodiscard]] std::weak_ptr<Pickable> GetPickableFromId(uint32_t id) const;
  [[nodiscard]] bool IsDragging() const;

  [[nodiscard]] Color GetPickableColor(std::weak_ptr<Pickable> pickable,
                                       BatcherId batcher_id);

  [[nodiscard]] bool IsThisElementPicked(const Pickable* pickable) const;

 private:
  [[nodiscard]] PickingId CreatePickableId(std::weak_ptr<Pickable> a_Pickable,
                                           BatcherId batcher_id);
  [[nodiscard]] Color ColorFromPickingID(PickingId id) const;

 private:
  uint32_t id_counter_ = 0;
  std::unordered_map<uint32_t, std::weak_ptr<Pickable>> id_pickable_map_;
  std::weak_ptr<Pickable> currently_picked_;
  mutable absl::Mutex mutex_;
};

#endif
