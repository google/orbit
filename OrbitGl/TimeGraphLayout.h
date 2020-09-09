// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIME_GRAPH_LAYOUT_H_
#define ORBIT_GL_TIME_GRAPH_LAYOUT_H_

class TimeGraphLayout {
 public:
  TimeGraphLayout();

  float GetTextBoxHeight() const { return m_TextBoxHeight * scale_; }
  float GetTextCoresHeight() const { return m_CoresHeight * scale_; }
  float GetEventTrackHeight() const { return m_EventTrackHeight * scale_; }
  float GetGraphTrackHeight() const { return m_GraphTrackHeight * scale_; }
  float GetTrackBottomMargin() const { return m_TrackBottomMargin * scale_; }
  float GetTrackTopMargin() const { return m_TrackTopMargin * scale_; }
  float GetTrackLabelOffsetX() const { return m_TrackLabelOffsetX; }
  float GetSliderWidth() const { return m_SliderWidth; }
  float GetTimeBarHeight() const { return time_bar_height_; }
  float GetTrackTabWidth() const { return m_TrackTabWidth; }
  float GetTrackTabHeight() const { return m_TrackTabHeight * scale_; }
  float GetTrackTabOffset() const { return m_TrackTabOffset; }
  float GetCollapseButtonOffset() const { return m_CollapseButtonOffset; }
  float GetRoundingRadius() const { return m_RoundingRadius * scale_; }
  float GetRoundingNumSides() const { return m_RoundingNumSides; }
  float GetTextOffset() const { return m_TextOffset; }
  float GetBottomMargin() const;
  float GetTopMargin() const { return GetSchedulerTrackOffset(); }
  float GetRightMargin() const { return right_margin_; }
  float GetSchedulerTrackOffset() const { return m_SchedulerTrackOffset * scale_; }
  float GetSpaceBetweenTracks() const { return m_SpaceBetweenTracks * scale_; }
  float GetSpaceBetweenCores() const { return m_SpaceBetweenCores * scale_; }
  float GetSpaceBetweenTracksAndThread() const { return m_SpaceBetweenTracksAndThread * scale_; }
  float GetToolbarIconHeight() const { return m_ToolbarIconHeight; }
  float GetScale() const { return scale_; }
  void SetScale(float value) { scale_ = value; }
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
  float m_SliderWidth;
  float time_bar_height_;
  float m_TrackTabWidth;
  float m_TrackTabHeight;
  float m_TrackTabOffset;
  float m_CollapseButtonOffset;
  float m_RoundingRadius;
  float m_RoundingNumSides;
  float m_TextOffset;
  float right_margin_;
  float m_SchedulerTrackOffset;

  float m_SpaceBetweenCores;
  float m_SpaceBetweenTracks;
  float m_SpaceBetweenTracksAndThread;
  float m_SpaceBetweenThreadBlocks;

  float m_ToolbarIconHeight;
  float scale_;

  bool m_DrawProperties = false;
  bool m_DrawTrackBackground = true;
};

#endif  // ORBIT_GL_TIME_GRAPH_LAYOUT_H_
