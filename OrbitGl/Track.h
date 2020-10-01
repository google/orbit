// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include "Batcher.h"
#include "BlockChain.h"
#include "CoreMath.h"
#include "OrbitBase/Profiling.h"
#include "PickingManager.h"
#include "TextBox.h"
#include "TextRenderer.h"
#include "TimeGraphLayout.h"
#include "TimerChain.h"
#include "TriangleToggle.h"

class GlCanvas;
class TimeGraph;

class Track : public Pickable, public std::enable_shared_from_this<Track> {
 public:
  enum Type {
    kTimerTrack,
    kThreadTrack,
    kEventTrack,
    kFrameTrack,
    kGraphTrack,
    kGpuTrack,
    kSchedulerTrack,
    kAsyncTrack,
    kUnknown,
  };

  explicit Track(TimeGraph* time_graph);
  ~Track() override = default;

  // Pickable
  void Draw(GlCanvas* a_Canvas, PickingMode a_PickingMode) override;
  virtual void UpdatePrimitives(uint64_t min_tick, uint64_t max_tick, PickingMode picking_mode);
  void OnPick(int a_X, int a_Y) override;
  void OnRelease() override;
  void OnDrag(int a_X, int a_Y) override;
  [[nodiscard]] bool Draggable() override { return true; }
  [[nodiscard]] bool Movable() override { return true; }

  [[nodiscard]] virtual Type GetType() const = 0;

  [[nodiscard]] virtual float GetHeight() const { return 0.f; };
  [[nodiscard]] bool GetVisible() const { return visible_; }
  void SetVisible(bool value) { visible_ = value; }

  [[nodiscard]] uint32_t GetNumTimers() const { return num_timers_; }
  [[nodiscard]] uint64_t GetMinTime() const { return min_time_; }
  [[nodiscard]] uint64_t GetMaxTime() const { return max_time_; }

  [[nodiscard]] virtual std::vector<std::shared_ptr<TimerChain>> GetTimers() { return {}; }
  [[nodiscard]] virtual std::vector<std::shared_ptr<TimerChain>> GetAllChains() { return {}; }
  [[nodiscard]] virtual std::vector<std::shared_ptr<TimerChain>> GetAllSerializableChains() {
    return {};
  }

  [[nodiscard]] bool IsMoving() const { return moving_; }
  [[nodiscard]] Vec2 GetMoveDelta() const {
    return moving_ ? mouse_pos_[1] - mouse_pos_[0] : Vec2(0, 0);
  }
  void SetName(const std::string& name) { name_ = name; }
  [[nodiscard]] const std::string& GetName() const { return name_; }
  void SetLabel(const std::string& label) { label_ = label; }
  [[nodiscard]] const std::string& GetLabel() const { return label_; }

  void SetTimeGraph(TimeGraph* timegraph) { time_graph_ = timegraph; }
  void SetPos(float a_X, float a_Y);
  void SetY(float y);
  [[nodiscard]] Vec2 GetPos() const { return pos_; }
  void SetSize(float a_SizeX, float a_SizeY);
  void SetColor(Color a_Color) { color_ = a_Color; }

  void AddChild(std::shared_ptr<Track> track) { children_.emplace_back(track); }
  virtual void OnCollapseToggle(TriangleToggle::State state);
  [[nodiscard]] virtual bool IsCollapsable() const { return false; }

 protected:
  void DrawTriangleFan(Batcher* batcher, const std::vector<Vec2>& points, const Vec2& pos,
                       const Color& color, float rotation, float z);

  [[nodiscard]] virtual bool IsTrackSelected() const { return false; }

  GlCanvas* canvas_;
  TimeGraph* time_graph_;
  Vec2 pos_;
  Vec2 size_;
  Vec2 mouse_pos_[2];
  Vec2 picking_offset_;
  bool picked_;
  bool moving_;
  std::string name_;
  std::string label_;
  TextBox thread_name_;
  int32_t thread_id_;
  Color color_;
  bool visible_ = true;
  std::atomic<uint32_t> num_timers_;
  std::atomic<uint64_t> min_time_;
  std::atomic<uint64_t> max_time_;
  bool picking_enabled_ = false;
  Type type_ = kUnknown;
  std::vector<std::shared_ptr<Track>> children_;
  std::shared_ptr<TriangleToggle> collapse_toggle_;
};
