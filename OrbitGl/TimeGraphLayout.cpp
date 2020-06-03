// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TimeGraphLayout.h"

#include "Capture.h"
#include "ImGuiOrbit.h"

//-----------------------------------------------------------------------------
TimeGraphLayout::TimeGraphLayout() {
  m_NumCores = 0;
  m_TextBoxHeight = 20.f;
  m_CoresHeight = 10.f;
  m_EventTrackHeight = 10.f;
  m_GraphTrackHeight = 20.f;
  m_TrackBottomMargin = 5.f;
  m_TrackTopMargin = 5.f;
  m_SpaceBetweenCores = 2.f;
  m_SpaceBetweenTracks = 40.f;
  m_SpaceBetweenTracksAndThread = 5.f;
  m_SpaceBetweenThreadBlocks = 35.f;
  m_TrackLabelOffsetX = 30.f;
  m_TrackLabelOffsetY = 10.f;
  m_SliderWidth = 15.f;
  m_TrackTabWidth = 350.f;
  m_TrackTabHeight = 30.f;
  m_TrackTabOffset = 0.f;
  m_CollapseButtonOffset = 15.f;
  m_RoundingRadius = 8.f;
  m_RoundingNumSides = 16;
  m_TextOffset = 5.f;
  m_VerticalMargin = 10.f;
  m_SchedulerTrackOffset = 62.f;
  m_TextZ = -0.02f;
  m_TrackZ = -0.1f;
  m_ToolbarIconHeight = 24.f;
};

//-----------------------------------------------------------------------------
#define FLOAT_SLIDER_MIN_MAX(x, min, max)     \
  if (ImGui::SliderFloat(#x, &x, min, max)) { \
    needs_redraw = true;                      \
  }

//-----------------------------------------------------------------------------
#define FLOAT_SLIDER(x) FLOAT_SLIDER_MIN_MAX(x, 0, 100.f)

//-----------------------------------------------------------------------------
bool TimeGraphLayout::DrawProperties() {
  ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
  ImVec2 size(400, 400);

  ImGui::Begin("Layout Properties", &m_DrawProperties, size, 1.f, 0);
  bool needs_redraw = false;
  FLOAT_SLIDER(m_TrackLabelOffsetX);
  FLOAT_SLIDER(m_TrackLabelOffsetY);
  FLOAT_SLIDER(m_TextBoxHeight);
  FLOAT_SLIDER(m_CoresHeight);
  FLOAT_SLIDER(m_EventTrackHeight);
  FLOAT_SLIDER(m_GraphTrackHeight);
  FLOAT_SLIDER(m_SpaceBetweenCores);
  FLOAT_SLIDER(m_SpaceBetweenTracks);
  FLOAT_SLIDER(m_SpaceBetweenTracksAndThread);
  FLOAT_SLIDER(m_SpaceBetweenThreadBlocks);
  FLOAT_SLIDER(m_SliderWidth);
  FLOAT_SLIDER(m_TrackTabHeight);
  FLOAT_SLIDER(m_TrackTabOffset);
  FLOAT_SLIDER(m_CollapseButtonOffset);
  FLOAT_SLIDER(m_RoundingRadius);
  FLOAT_SLIDER(m_RoundingNumSides);
  FLOAT_SLIDER(m_TextOffset);
  FLOAT_SLIDER(m_VerticalMargin);
  FLOAT_SLIDER(m_SchedulerTrackOffset);
  FLOAT_SLIDER_MIN_MAX(m_TrackTabWidth, 0, 1000.f);
  FLOAT_SLIDER_MIN_MAX(m_TrackBottomMargin, 0, 20.f);
  FLOAT_SLIDER_MIN_MAX(m_TrackTopMargin, 0, 20.f);
  FLOAT_SLIDER_MIN_MAX(m_TextZ, -1.f, 1.f);
  FLOAT_SLIDER_MIN_MAX(m_TrackZ, -1.f, 1.f);
  FLOAT_SLIDER(m_ToolbarIconHeight);
  ImGui::Checkbox("DrawTrackBackground", &m_DrawTrackBackground);
  ImGui::End();

  return needs_redraw;
}
