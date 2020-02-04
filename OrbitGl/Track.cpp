//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Track.h"

#include "absl/strings/str_format.h"

#include "Capture.h"
#include "GlCanvas.h"
#include "TimeGraphLayout.h"

float TEXT_Z = -0.004f;
float TRACK_Z = -0.005f;

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
}

//-----------------------------------------------------------------------------
void Track::Draw(GlCanvas* a_Canvas, bool a_Picking) {
  static volatile unsigned char alpha = 255;
  static volatile unsigned char grey = 60;
  auto col = Color(grey, grey, grey, alpha);
  a_Picking ? PickingManager::SetPickingColor(
                  a_Canvas->GetPickingManager().CreatePickableId(this))
            : glColor4ubv(&col[0]);

  float x0 = m_Pos[0];
  float x1 = x0 + m_Size[0];

  float y0 = m_Pos[1];
  float y1 = y0 - m_Size[1];

  if (m_Picked) {
    glColor4ub(0, 128, 255, 128);
  }

  glBegin(GL_QUADS);
  glVertex3f(x0, y0, TRACK_Z);
  glVertex3f(x1, y0, TRACK_Z);
  glVertex3f(x1, y1, TRACK_Z);
  glVertex3f(x0, y1, TRACK_Z);
  glEnd();

  if (a_Canvas->GetPickingManager().GetPicked() == this)
    glColor4ub(255, 255, 255, 255);
  else
    glColor4ubv(&col[0]);

  glBegin(GL_LINES);
  glVertex3f(x0, y0, TRACK_Z);
  glVertex3f(x1, y0, TRACK_Z);
  glVertex3f(x1, y1, TRACK_Z);
  glVertex3f(x0, y1, TRACK_Z);
  glEnd();

  std::string name = absl::StrFormat("%s [%u]", m_Name.c_str(), m_ID);
  a_Canvas->AddText(name.c_str(), x0, y1, TEXT_Z, Color(255, 255, 255, 255));

  m_Canvas = a_Canvas;
}

//-----------------------------------------------------------------------------
void Track::SetPos(float a_X, float a_Y) {
  if (!m_Moving) {
    m_Pos = Vec2(a_X, a_Y);
  }
}

//-----------------------------------------------------------------------------
void Track::SetSize(float a_SizeX, float a_SizeY) {
  m_Size = Vec2(a_SizeX, a_SizeY);
}

//-----------------------------------------------------------------------------
void Track::OnPick(int a_X, int a_Y) {
  Vec2& mousePos = m_MousePos[0];
  m_Canvas->ScreenToWorld(a_X, a_Y, mousePos[0], mousePos[1]);
  m_PickingOffset = mousePos - m_Pos;
  m_MousePos[1] = m_MousePos[0];
  m_Picked = true;
}

//-----------------------------------------------------------------------------
void Track::OnRelease() {
  m_Picked = false;
  m_Moving = false;
  m_TimeGraph->NeedsUpdate();
}

//-----------------------------------------------------------------------------
void Track::OnDrag(int a_X, int a_Y) {
  m_Moving = true;
  float x = 0.f;
  m_Canvas->ScreenToWorld(a_X, a_Y, x, m_Pos[1]);
  m_MousePos[1] = m_Pos;
  m_Pos[1] -= m_PickingOffset[1];
  m_TimeGraph->NeedsUpdate();
}
