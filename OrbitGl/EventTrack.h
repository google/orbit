//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
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

  void Draw(GlCanvas* a_Canvas, bool a_Picking) override;
  void UpdatePrimitives(uint64_t min_tick, uint64_t max_tick) override;

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
  void SelectEvents();

 protected:
  TextBox m_ThreadName;
  GlCanvas* m_Canvas;
  ThreadID m_ThreadId;
  TimeGraph* time_graph_;
  Vec2 m_Pos;
  Vec2 m_Size;
  Vec2 m_MousePos[2];
  bool m_Picked;
  Color m_Color;
  std::vector<CallstackEvent> selected_callstack_events_;
};
