// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstring>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "CoreMath.h"
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
  virtual bool Draggable() { return false; }
  virtual bool Movable() { return false; }
  virtual std::string GetTooltip() const { return ""; }
};

struct PickingID {
  enum Type {
    kInvalid,
    kLine,
    kBox,
    kTriangle,
    kPickable,
  };

  // Instances of batchers used to draw must be in 1:1 correspondence with
  // values in the following enum. Currently, two batchers are used, one to draw
  // UI elements (corresponding to BatcherId::UI), and one for drawing events on
  // the time graph (corresponding to BatcherId::kTimeGraph). If you want to add
  // more batchers, this enum must be extended and you need to spend more bits
  // on the batcher_id_ field below. The total number of elements that can be
  // correctly picked is limited to the number of elements that can be encoded
  // in the bits remaining after encoding the batcher id, and the
  // PickingID::Type, so adding more batchers or types has to be carefully
  // considered.
  enum BatcherId { kTimeGraph, kUi };

  static PickingID Get(Type type, uint32_t id,
                       BatcherId batcher_id = BatcherId::kTimeGraph) {
    static_assert(sizeof(PickingID) == 4, "PickingID must be 32 bits");
    PickingID result;
    result.type_ = type;
    result.id_ = id;
    result.batcher_id_ = batcher_id;
    return result;
  }
  static Color GetColor(Type type, uint32_t id,
                        BatcherId batcher_id = BatcherId::kTimeGraph) {
    static_assert(sizeof(PickingID) == sizeof(Color),
                  "PickingId and Color must have the same size");
    static_assert(std::is_trivially_copyable<PickingID>::value,
                  "PickingID must be trivially copyable");

    PickingID result_id = Get(type, id, batcher_id);
    std::array<uint8_t, 4> color_values;
    std::memcpy(&color_values[0], &result_id, sizeof(PickingID));

    return Color(color_values[0], color_values[1], color_values[2],
                 color_values[3]);
  }
  static PickingID Get(uint32_t value) {
    static_assert(sizeof(PickingID) == sizeof(uint32_t),
                  "PickingId and uint32_t must have the same size");
    static_assert(std::is_trivially_copyable<PickingID>::value,
                  "PickingID must be trivially copyable");

    PickingID id;
    std::memcpy(&id, &value, sizeof(uint32_t));
    return id;
  }
  uint32_t id_ : 28;
  uint32_t type_ : 3;
  uint32_t batcher_id_ : 1;
};

class PickingManager {
 public:
  PickingManager() {}
  void Reset();

  void Pick(uint32_t a_Id, int a_X, int a_Y);
  void Release();
  void Drag(int a_X, int a_Y);
  std::weak_ptr<Pickable> GetPicked() const;
  std::weak_ptr<Pickable> GetPickableFromId(uint32_t id) const;
  bool IsDragging() const;

  Color GetPickableColor(std::weak_ptr<Pickable> pickable,
                         PickingID::BatcherId batcher_id);

  bool IsThisElementPicked(const Pickable* pickable) const;

 private:
  PickingID CreatePickableId(std::weak_ptr<Pickable> a_Pickable,
                             PickingID::BatcherId batcher_id);
  Color ColorFromPickingID(PickingID id) const;

 private:
  std::vector<Pickable*> m_Pickables;
  uint32_t m_IdCounter = 0;
  std::unordered_map<uint32_t, std::weak_ptr<Pickable>> m_IdPickableMap;
  std::weak_ptr<Pickable> m_Picked;
  mutable absl::Mutex mutex_;
};
