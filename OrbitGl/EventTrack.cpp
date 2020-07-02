// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EventTrack.h"

#include "Capture.h"
#include "EventTracer.h"
#include "GlCanvas.h"

//-----------------------------------------------------------------------------
EventTrack::EventTrack(TimeGraph* a_TimeGraph) : Track(a_TimeGraph) {
  m_MousePos[0] = m_MousePos[1] = Vec2(0, 0);
  m_Picked = false;
  m_Color = Color(0, 255, 0, 255);
}

//-----------------------------------------------------------------------------
void EventTrack::Draw(GlCanvas* canvas, bool picking) {
  Batcher* batcher = canvas->GetBatcher();
  batcher->Reset();
  PickingManager& picking_manager = canvas->GetPickingManager();

  constexpr float kNormalZ = -0.1f;
  constexpr float kPickingZ = 0.1f;
  float z = kNormalZ;
  Color color = m_Color;

  if (picking) {
    z = kPickingZ;
    color = picking_manager.GetPickableColor(this);
  }

  Box box(m_Pos, Vec2(m_Size[0], -m_Size[1]), z);
  batcher->AddBox(box, color, PickingID::BOX);

  if (canvas->GetPickingManager().GetPicked() == this) {
    color = Color(255, 255, 255, 255);
  }

  float x0 = m_Pos[0];
  float y0 = m_Pos[1];
  float x1 = x0 + m_Size[0];
  float y1 = y0 - m_Size[1];

  batcher->AddLine(m_Pos, Vec2(x1, y0), -0.1f, color, PickingID::LINE);
  batcher->AddLine(Vec2(x1, y1), Vec2(x0, y1), -0.1f, color, PickingID::LINE);

  if (m_Picked) {
    Vec2& from = m_MousePos[0];
    Vec2& to = m_MousePos[1];

    x0 = from[0];
    y0 = m_Pos[1];
    x1 = to[0];
    y1 = y0 - m_Size[1];

    Color picked_color(0, 128, 255, 128);
    Box box(Vec2(x0, y0), Vec2(x1 - x0, -m_Size[1]), -0.f);
    batcher->AddBox(box, picked_color, PickingID::BOX);
  }

  m_Canvas = canvas;

  batcher->Draw();
  batcher->Reset();
}

//-----------------------------------------------------------------------------
void EventTrack::UpdatePrimitives(uint64_t min_tick, uint64_t max_tick) {
  Batcher* batcher = &time_graph_->GetBatcher();
  const TimeGraphLayout& layout = time_graph_->GetLayout();
  float z = GlCanvas::Z_VALUE_EVENT;
  float track_height = layout.GetEventTrackHeight();

  ScopeLock lock(GEventTracer.GetEventBuffer().GetMutex());
  std::map<uint64_t, CallstackEvent>& callstacks =
      GEventTracer.GetEventBuffer().GetCallstacks()[m_ThreadId];

  // Sampling Events
  const Color kWhite(255, 255, 255, 255);
  for (auto& pair : callstacks) {
    uint64_t time = pair.first;
    if (time > min_tick && time < max_tick) {
      Vec2 pos(time_graph_->GetWorldFromTick(time), m_Pos[1]);
      batcher->AddVerticalLine(pos, -track_height, z, kWhite, PickingID::EVENT);
    }
  }

  // Draw selected events
  const Color kGreenSelection(0, 255, 0, 255);
  Color selectedColor[2];
  Fill(selectedColor, kGreenSelection);
  for (CallstackEvent& event : selected_callstack_events_) {
    Vec2 pos(time_graph_->GetWorldFromTick(event.m_Time), m_Pos[1]);
    batcher->AddVerticalLine(pos, -track_height, z, kGreenSelection,
                             PickingID::EVENT);
  }
}

//-----------------------------------------------------------------------------
void EventTrack::SetPos(float a_X, float a_Y) {
  m_Pos = Vec2(a_X, a_Y);
  m_ThreadName.SetPos(Vec2(a_X, a_Y));
  m_ThreadName.SetSize(Vec2(m_Size[0] * 0.3f, m_Size[1]));
}

//-----------------------------------------------------------------------------
void EventTrack::SetSize(float a_SizeX, float a_SizeY) {
  m_Size = Vec2(a_SizeX, a_SizeY);
}

//-----------------------------------------------------------------------------
void EventTrack::OnPick(int a_X, int a_Y) {
  Capture::GSelectedThreadId = m_ThreadId;
  Vec2& mousePos = m_MousePos[0];
  m_Canvas->ScreenToWorld(a_X, a_Y, mousePos[0], mousePos[1]);
  m_MousePos[1] = m_MousePos[0];
  m_Picked = true;
}

//-----------------------------------------------------------------------------
void EventTrack::OnRelease() {
  if (m_Picked) {
    SelectEvents();
  }

  m_Picked = false;
}

//-----------------------------------------------------------------------------
void EventTrack::OnDrag(int a_X, int a_Y) {
  Vec2& to = m_MousePos[1];
  m_Canvas->ScreenToWorld(a_X, a_Y, to[0], to[1]);
}

//-----------------------------------------------------------------------------
void EventTrack::SelectEvents() {
  Vec2& from = m_MousePos[0];
  Vec2& to = m_MousePos[1];

  selected_callstack_events_ =
      time_graph_->SelectEvents(from[0], to[0], m_ThreadId);
}

//-----------------------------------------------------------------------------
bool EventTrack::IsEmpty() const {
  ScopeLock lock(GEventTracer.GetEventBuffer().GetMutex());
  const std::map<uint64_t, CallstackEvent>& callstacks =
      GEventTracer.GetEventBuffer().GetCallstacks()[m_ThreadId];
  return callstacks.empty();
}
