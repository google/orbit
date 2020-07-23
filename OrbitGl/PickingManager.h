// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstring>
#include <unordered_map>
#include <vector>

#include "CoreMath.h"
#include "absl/synchronization/mutex.h"

class GlCanvas;

//-----------------------------------------------------------------------------
enum class PickingMode {
  kNone,
  kHover,
  kClick
};

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

//-----------------------------------------------------------------------------
struct PickingID {
  enum Type {
    INVALID,
    LINE,
    BOX,
    TRIANGLE,
    PICKABLE,
  };

  // Instances of batchers used to draw must be in 1:1 correspondence with
  // values in the following enum. Currently, two batchers are used, one to draw
  // UI elements (corresponding to BatcherId::UI), and one for drawing events on
  // the time graph (corresponding to BatcherId::TIME_GRAPH). If you want to add
  // more batchers, this enum must be extended and you need to spend more bits
  // on the batcher_id_ field below. The total number of elements that can be
  // correctly picked is limited to the number of elements that can be encoded
  // in the bits remaining after encoding the batcher id, and the
  // PickingID::Type, so adding more batchers or types has to be carefully
  // considered.
  enum BatcherId { TIME_GRAPH, UI };

  static PickingID Get(Type a_Type, uint32_t a_ID,
                       BatcherId batcher_id = TIME_GRAPH) {
    static_assert(sizeof(PickingID) == 4, "PickingID must be 32 bits");
    PickingID id;
    id.m_Type = a_Type;
    id.m_Id = a_ID;
    id.batcher_id_ = batcher_id;
    return id;
  }
  static Color GetColor(Type a_Type, uint32_t a_ID,
                        BatcherId batcher_id = TIME_GRAPH) {
    static_assert(sizeof(PickingID) == sizeof(Color),
                  "PickingId and Color must have the same size");
    static_assert(std::is_trivially_copyable<PickingID>::value,
                  "PickingID must be trivially copyable");

    PickingID id = Get(a_Type, a_ID, batcher_id);
    std::array<uint8_t, 4> color_values;
    std::memcpy(&color_values[0], &id, sizeof(PickingID));

    return Color(color_values[0], color_values[1], color_values[2],
                 color_values[3]);
  }
  static PickingID Get(uint32_t a_Value) {
    static_assert(sizeof(PickingID) == sizeof(uint32_t),
                  "PickingId and uint32_t must have the same size");
    static_assert(std::is_trivially_copyable<PickingID>::value,
                  "PickingID must be trivially copyable");

    PickingID id;
    std::memcpy(&id, &a_Value, sizeof(uint32_t));
    return id;
  }
  uint32_t m_Id : 28;
  uint32_t m_Type : 3;
  uint32_t batcher_id_ : 1;
};

//-----------------------------------------------------------------------------
class PickingManager {
 public:
  PickingManager() {}
  void Reset();

  void Pick(uint32_t a_Id, int a_X, int a_Y);
  void Release();
  void Drag(int a_X, int a_Y);
  Pickable* GetPicked();
  Pickable* GetPickableFromId(uint32_t id);
  bool IsDragging() const;

  Color GetPickableColor(Pickable* pickable, PickingID::BatcherId batcher_id);

 private:
  PickingID CreatePickableId(Pickable* a_Pickable,
                             PickingID::BatcherId batcher_id);
  Color ColorFromPickingID(PickingID id) const;

 private:
  std::vector<Pickable*> m_Pickables;
  uint32_t m_IdCounter = 0;
  std::unordered_map<Pickable*, uint32_t> m_PickableIdMap;
  std::unordered_map<uint32_t, Pickable*> m_IdPickableMap;
  Pickable* m_Picked = nullptr;
  mutable absl::Mutex mutex_;
};
