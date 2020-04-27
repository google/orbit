//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "EventTrack.h"

#include "Capture.h"
#include "EventTracer.h"
#include "GlCanvas.h"

//-----------------------------------------------------------------------------
EventTrack::EventTrack(TimeGraph* a_TimeGraph) : time_graph_(a_TimeGraph) {
  m_MousePos[0] = m_MousePos[1] = Vec2(0, 0);
  m_Picked = false;
  m_Color = Color(0, 255, 0, 255);
}

//-----------------------------------------------------------------------------
void EventTrack::Draw(GlCanvas* a_Canvas, bool a_Picking) {
  Color col = m_Color;

  a_Picking ? PickingManager::SetPickingColor(
                  a_Canvas->GetPickingManager().CreatePickableId(this))
            : glColor4ubv(&col[0]);

  float x0 = m_Pos[0];
  float y0 = m_Pos[1];
  float x1 = x0 + m_Size[0];
  float y1 = y0 - m_Size[1];

  glBegin(GL_QUADS);
  glVertex3f(x0, y0, -0.1f);
  glVertex3f(x1, y0, -0.1f);
  glVertex3f(x1, y1, -0.1f);
  glVertex3f(x0, y1, -0.1f);
  glEnd();

  if (a_Canvas->GetPickingManager().GetPicked() == this)
    glColor4ub(255, 255, 255, 255);
  else
    glColor4ubv(&col[0]);

  glBegin(GL_LINES);
  glVertex3f(x0, y0, -0.1f);
  glVertex3f(x1, y0, -0.1f);
  glVertex3f(x1, y1, -0.1f);
  glVertex3f(x0, y1, -0.1f);
  glEnd();

  if (m_Picked) {
    Vec2& from = m_MousePos[0];
    Vec2& to = m_MousePos[1];

    x0 = from[0];
    y0 = m_Pos[1];
    x1 = to[0];
    y1 = y0 - m_Size[1];

    glColor4ub(0, 128, 255, 128);
    glBegin(GL_QUADS);
    glVertex3f(x0, y0, -0.f);
    glVertex3f(x1, y0, -0.f);
    glVertex3f(x1, y1, -0.f);
    glVertex3f(x0, y1, -0.f);
    glEnd();
  }

  m_Canvas = a_Canvas;
}

//-----------------------------------------------------------------------------
void EventTrack::UpdatePrimitives(uint64_t min_tick, uint64_t max_tick) {
  Batcher* batcher = &time_graph_->GetBatcher();
  const TimeGraphLayout& layout = time_graph_->GetLayout();

  Color lineColor[2];
  Color white(255, 255, 255, 255);
  Fill(lineColor, white);

  ScopeLock lock(GEventTracer.GetEventBuffer().GetMutex());
  std::map<uint64_t, CallstackEvent>& callstacks =
      GEventTracer.GetEventBuffer().GetCallstacks()[m_ThreadId];

  // Sampling Events
  for (auto& pair : callstacks) {
    uint64_t time = pair.first;
    if (time > min_tick && time < max_tick) {
      float x = time_graph_->GetWorldFromTick(time);
      Line line;
      line.m_Beg = Vec3(x, m_Pos[1], GlCanvas::Z_VALUE_EVENT);
      line.m_End = Vec3(x, m_Pos[1] - layout.GetEventTrackHeight(),
                        GlCanvas::Z_VALUE_EVENT);
      batcher->AddLine(line, lineColor, PickingID::EVENT);
    }
  }

  // Draw selected events
  const Color kGreenSelection(0, 255, 0, 255);
  Color selectedColor[2];
  Fill(selectedColor, kGreenSelection);
  for (CallstackEvent& event : selected_callstack_events_) {
    float x = time_graph_->GetWorldFromTick(event.m_Time);
    Line line;
    line.m_Beg = Vec3(x, m_Pos[1], GlCanvas::Z_VALUE_EVENT);
    line.m_End = Vec3(x, m_Pos[1] - layout.GetEventTrackHeight(),
                      GlCanvas::Z_VALUE_TEXT);
    batcher->AddLine(line, selectedColor, PickingID::EVENT);
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
