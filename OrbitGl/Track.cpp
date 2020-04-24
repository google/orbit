//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Track.h"

#include "Capture.h"
#include "GlCanvas.h"
#include "TimeGraphLayout.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
Track::Track() {
  m_ID = 0;
  m_MousePos[0] = m_MousePos[1] = Vec2(0, 0);
  m_Pos = Vec2(0, 0);
  m_Size = Vec2(0, 0);
  m_PickingOffset = Vec2(0, 0);
  m_Picked = false;
  m_Moving = false;
  m_Canvas = nullptr;

  label_display_mode_ = NAME_AND_TID;

  unsigned char alpha = 255;
  unsigned char grey = 60;
  m_Color = Color(grey, grey, grey, alpha);
}

//-----------------------------------------------------------------------------
void Track::Draw(GlCanvas* a_Canvas, bool a_Picking) {
  static volatile unsigned char alpha = 255;
  static volatile unsigned char grey = 60;
  auto col = Color(grey, grey, grey, alpha);
  a_Picking ? PickingManager::SetPickingColor(
                  a_Canvas->GetPickingManager().CreatePickableId(this))
            : glColor4ubv(&m_Color[0]);

  float x0 = m_Pos[0];
  float x1 = x0 + m_Size[0];

  float y0 = m_Pos[1];
  float y1 = y0 - m_Size[1];

  if (m_Picked) {
    glColor4ub(0, 128, 255, 128);
  }

  const TimeGraphLayout& layout = time_graph_->GetLayout();
  float track_z = layout.GetTrackZ();
  float text_z = layout.GetTextZ();
  float label_offset = layout.GetTrackLabelOffset();

  if (layout.GetDrawTrackBackground()) {
    glBegin(GL_QUADS);
    glVertex3f(x0, y0, track_z);
    glVertex3f(x1, y0, track_z);
    glVertex3f(x1, y1, track_z);
    glVertex3f(x0, y1, track_z);
    glEnd();
  }

  if (a_Canvas->GetPickingManager().GetPicked() == this)
    glColor4ub(255, 255, 255, 255);
  else
    glColor4ubv(&m_Color[0]);

  glBegin(GL_LINES);
  glVertex3f(x0, y0, track_z);
  glVertex3f(x1, y0, track_z);
  glVertex3f(x1, y1, track_z);
  glVertex3f(x0, y1, track_z);
  glEnd();

  std::string track_label;
  switch (label_display_mode_) {
    case NAME_AND_TID:
      track_label = absl::StrFormat("%s [%u]", m_Name, m_ID);
      break;
    case TID_ONLY:
      track_label = absl::StrFormat("[%u]", m_ID);
      break;
    case NAME_ONLY:
      track_label = absl::StrFormat("%s", m_Name);
      break;
    case EMPTY:
      track_label = "";
  }

  a_Canvas->AddText(track_label.c_str(), x0, y1 + label_offset + m_Size[1],
                    text_z, Color(255, 255, 255, 255));

  m_Canvas = a_Canvas;
}

//-----------------------------------------------------------------------------
void Track::UpdatePrimitives(uint64_t /*t_min*/, uint64_t /*t_max*/) {}

//-----------------------------------------------------------------------------
void Track::SetPos(float a_X, float a_Y) {
  if (!m_Moving) {
    m_Pos = Vec2(a_X, a_Y);
  }
}

//-----------------------------------------------------------------------------
void Track::SetY(float y) {
  if (!m_Moving) {
    m_Pos[1] = y;
  }
}

//-----------------------------------------------------------------------------
void Track::SetSize(float a_SizeX, float a_SizeY) {
  m_Size = Vec2(a_SizeX, a_SizeY);
}

//-----------------------------------------------------------------------------
void Track::OnPick(int a_X, int a_Y) {
  if (!m_PickingEnabled) return;

  Vec2& mousePos = m_MousePos[0];
  m_Canvas->ScreenToWorld(a_X, a_Y, mousePos[0], mousePos[1]);
  m_PickingOffset = mousePos - m_Pos;
  m_MousePos[1] = m_MousePos[0];
  m_Picked = true;
}

//-----------------------------------------------------------------------------
void Track::OnRelease() {
  if (!m_PickingEnabled) return;

  m_Picked = false;
  m_Moving = false;
  time_graph_->NeedsUpdate();
}

//-----------------------------------------------------------------------------
void Track::OnDrag(int a_X, int a_Y) {
  if (!m_PickingEnabled) return;

  m_Moving = true;
  float x = 0.f;
  m_Canvas->ScreenToWorld(a_X, a_Y, x, m_Pos[1]);
  m_MousePos[1] = m_Pos;
  m_Pos[1] -= m_PickingOffset[1];
  time_graph_->NeedsUpdate();
}
