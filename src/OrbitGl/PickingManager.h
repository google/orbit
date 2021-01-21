// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_PICKING_MANAGER_H
#define ORBIT_GL_PICKING_MANAGER_H

#include <array>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "CoreMath.h"
#include "OrbitBase/Logging.h"
#include "absl/base/casts.h"
#include "absl/synchronization/mutex.h"

class GlCanvas;

enum class PickingMode { kNone, kHover, kClick };

class Pickable {
 public:
  virtual ~Pickable() = default;
  virtual void OnPick(int x, int y) = 0;
  virtual void OnDrag(int /*x*/, int /*y*/) {}
  virtual void OnRelease() {}
  [[nodiscard]] virtual bool Draggable() { return false; }
  [[nodiscard]] virtual std::string GetTooltip() const { return ""; }
};

enum class PickingType { kInvalid = 0, kLine = 1, kBox = 2, kTriangle = 3, kPickable = 4 };

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
enum class BatcherId { kTimeGraph, kUi };

struct PickingId {
  static constexpr uint32_t kElementIDBitSize = 28;
  static constexpr uint32_t kPickingTypeBitSize = 3;
  static constexpr uint32_t kBatcherIDBitSize = 1;

  struct Layout {
    uint32_t element_id : kElementIDBitSize;
    uint32_t type : kPickingTypeBitSize;
    uint32_t batcher_id : kBatcherIDBitSize;
  };

  static_assert(sizeof(Layout) == sizeof(uint32_t),
                "Layout needs to have the size of a single uint32_t.");

  [[nodiscard]] inline static PickingId Create(PickingType type, uint32_t element_id,
                                               BatcherId batcher_id = BatcherId::kTimeGraph) {
    CHECK(element_id >> kElementIDBitSize == 0);
    PickingId result{};
    result.type = type;
    result.element_id = element_id;
    result.batcher_id = batcher_id;
    return result;
  }

  [[nodiscard]] inline static PickingId FromPixelValue(uint32_t value) {
    const Layout layout = absl::bit_cast<Layout, uint32_t>(value);
    CHECK(layout.type >= static_cast<uint32_t>(PickingType::kInvalid) &&
          layout.type <= static_cast<uint32_t>(PickingType::kPickable));
    CHECK(layout.batcher_id >= static_cast<uint32_t>(BatcherId::kTimeGraph) &&
          layout.batcher_id <= static_cast<uint32_t>(BatcherId::kUi));

    PickingId id{};
    id.element_id = layout.element_id;
    id.type = static_cast<PickingType>(layout.type);
    id.batcher_id = static_cast<BatcherId>(layout.batcher_id);
    return id;
  }

  [[nodiscard]] uint32_t ToPixelValue() const {
    Layout layout{};
    layout.element_id = element_id;
    layout.type = static_cast<uint32_t>(type);
    layout.batcher_id = static_cast<uint32_t>(batcher_id);

    return absl::bit_cast<uint32_t>(layout);
  }

  [[nodiscard]] static Color ToColor(PickingType type, uint32_t element_id,
                                     BatcherId batcher_id = BatcherId::kTimeGraph) {
    uint32_t pixel_value = Create(type, element_id, batcher_id).ToPixelValue();
    std::array<uint8_t, 4> color_values{};
    color_values = absl::bit_cast<std::array<uint8_t, 4>>(pixel_value);
    return Color(color_values[0], color_values[1], color_values[2], color_values[3]);
  }

  uint32_t element_id;
  PickingType type;
  BatcherId batcher_id;
};

class PickingManager {
 public:
  PickingManager() = default;
  PickingManager(PickingManager& rhs) = delete;

  void Reset();

  void Pick(PickingId id, int x, int y);
  void Release();
  void Drag(int x, int y);
  [[nodiscard]] std::shared_ptr<Pickable> GetPicked() const;
  [[nodiscard]] std::shared_ptr<Pickable> GetPickableFromId(PickingId id) const;
  [[nodiscard]] bool IsDragging() const;

  [[nodiscard]] Color GetPickableColor(const std::shared_ptr<Pickable>& pickable,
                                       BatcherId batcher_id);

  [[nodiscard]] bool IsThisElementPicked(const Pickable* pickable) const;

 private:
  [[nodiscard]] PickingId GetOrCreatePickableId(const std::shared_ptr<Pickable>& pickable,
                                                BatcherId batcher_id);
  [[nodiscard]] Color ColorFromPickingID(PickingId id) const;

 private:
  uint32_t pickable_id_counter_ = 0;
  std::unordered_map<uint32_t, std::weak_ptr<Pickable>> pid_pickable_map_;
  std::unordered_map<Pickable*, uint32_t> pickable_pid_map_;
  std::weak_ptr<Pickable> currently_picked_;
  mutable absl::Mutex mutex_;
};

#endif
