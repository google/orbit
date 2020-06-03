// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIME_GRAPH_LAYOUT_H_
#define ORBIT_GL_TIME_GRAPH_LAYOUT_H_

//-----------------------------------------------------------------------------
class TimeGraphLayout {
 public:
  TimeGraphLayout();

  float GetTextBoxHeight() const { return m_TextBoxHeight; }
  float GetTextCoresHeight() const { return m_CoresHeight; }
  float GetEventTrackHeight() const { return m_EventTrackHeight; }
  float GetGraphTrackHeight() const { return m_GraphTrackHeight; }
  float GetTrackBottomMargin() const { return m_TrackBottomMargin; }
  float GetTrackTopMargin() const { return m_TrackTopMargin; }
  float GetTrackLabelOffsetX() const { return m_TrackLabelOffsetX; }
  float GetTrackLabelOffsetY() const { return m_TrackLabelOffsetY; }
  float GetSliderWidth() const { return m_SliderWidth; }
  float GetTrackTabWidth() const { return m_TrackTabWidth; }
  float GetTrackTabHeight() const { return m_TrackTabHeight; }
  float GetTrackTabOffset() const { return m_TrackTabOffset; }
  float GetCollapseButtonOffset() const { return m_CollapseButtonOffset; }
  float GetRoundingRadius() const { return m_RoundingRadius; }
  float GetRoundingNumSides() const { return m_RoundingNumSides; }
  float GetTextOffset() const { return m_TextOffset; }
  float GetVerticalMargin() const { return m_VerticalMargin; }
  float GetSchedulerTrackOffset() const { return m_SchedulerTrackOffset; }
  float GetSpaceBetweenTracks() const { return m_SpaceBetweenTracks; }
  float GetSpaceBetweenCores() const { return m_SpaceBetweenCores; }
  float GetSpaceBetweenTracksAndThread() const {
    return m_SpaceBetweenTracksAndThread;
  }
  float GetTextZ() const { return m_TextZ; }
  float GetTrackZ() const { return m_TrackZ; }
  float GetToolbarIconHeight() const { return m_ToolbarIconHeight; }
  void SetDrawProperties(bool value) { m_DrawProperties = value; }
  void SetNumCores(int a_NumCores) { m_NumCores = a_NumCores; }
  bool DrawProperties();
  bool GetDrawTrackBackground() const { return m_DrawTrackBackground; }

 protected:
  int m_NumCores;

  float m_TextBoxHeight;
  float m_CoresHeight;
  float m_EventTrackHeight;
  float m_GraphTrackHeight;
  float m_TrackBottomMargin;
  float m_TrackTopMargin;
  float m_TrackLabelOffsetX;
  float m_TrackLabelOffsetY;
  float m_SliderWidth;
  float m_TrackTabWidth;
  float m_TrackTabHeight;
  float m_TrackTabOffset;
  float m_CollapseButtonOffset;
  float m_RoundingRadius;
  float m_RoundingNumSides;
  float m_TextOffset;
  float m_VerticalMargin;
  float m_SchedulerTrackOffset;

  float m_SpaceBetweenCores;
  float m_SpaceBetweenTracks;
  float m_SpaceBetweenTracksAndThread;
  float m_SpaceBetweenThreadBlocks;

  float m_TextZ;
  float m_TrackZ;
  float m_ToolbarIconHeight;

  bool m_DrawProperties = false;
  bool m_DrawTrackBackground = true;
};

#endif  // ORBIT_GL_TIME_GRAPH_LAYOUT_H_
