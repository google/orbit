// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "CallstackTypes.h"
#include "EventBuffer.h"
#include "Track.h"

class GlCanvas;
class TimeGraph;

class EventTrack : public Track {
 public:
  explicit EventTrack(TimeGraph* a_TimeGraph);
  Type GetType() const override { return kEventTrack; }

  std::string GetTooltip() const override;

  void Draw(GlCanvas* canvas, PickingMode picking_mode) override;
  void UpdatePrimitives(uint64_t min_tick, uint64_t max_tick,
                        PickingMode picking_mode) override;

  void OnPick(int a_X, int a_Y) override;
  void OnRelease() override;
  void OnDrag(int a_X, int a_Y) override;
  bool Draggable() override { return true; }
  float GetHeight() const override { return m_Size[1]; }

  void SetThreadId(ThreadID a_ThreadId) { m_ThreadId = a_ThreadId; }
  void SetTimeGraph(TimeGraph* a_TimeGraph) { time_graph_ = a_TimeGraph; }
  void SetPos(float a_X, float a_Y);
  void SetSize(float a_SizeX, float a_SizeY);
  void SetColor(Color color) { m_Color = color; }
  bool IsEmpty() const;

 protected:
  void SelectEvents();
  std::string GetSampleTooltip(PickingID id) const;

 protected:
  TextBox m_ThreadName;
  GlCanvas* m_Canvas;
  ThreadID m_ThreadId;
  Vec2 m_Pos;
  Vec2 m_Size;
  Vec2 m_MousePos[2];
  bool m_Picked;
  Color m_Color;
};
