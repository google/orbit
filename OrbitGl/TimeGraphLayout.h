//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#ifndef ORBIT_GL_TIME_GRAPH_LAYOUT_H_
#define ORBIT_GL_TIME_GRAPH_LAYOUT_H_

//-----------------------------------------------------------------------------
class TimeGraphLayout {
 public:
  TimeGraphLayout();

  float GetCoreOffset(int a_CoreId) const;
  float GetThreadStart();
  float GetTextBoxHeight() const { return m_TextBoxHeight; }
  float GetTextCoresHeight() const { return m_CoresHeight; }
  float GetEventTrackHeight() const { return m_EventTrackHeight; }
  float GetGraphTrackHeight() const { return m_GraphTrackHeight; }
  float GetTrackBottomMargin() const { return m_TrackBottomMargin; }
  float GetTrackLabelOffset() const { return m_TrackLabelOffset; }
  float GetSliderWidth() const { return m_SliderWidth; }
  float GetSpaceBetweenTracks() const { return m_SpaceBetweenTracks; }
  float GetSpaceBetweenCores() const { return m_SpaceBetweenCores; }
  float GetSpaceBetweenTracksAndThread() const {
    return m_SpaceBetweenTracksAndThread;
  }
  float GetTextZ() const { return m_TextZ; }
  float GetTrackZ() const { return m_TrackZ; }
  void SetDrawProperties(bool value) { m_DrawProperties = value; }
  void SetNumCores(int a_NumCores) { m_NumCores = a_NumCores; }
  bool DrawProperties();
  bool GetDrawTrackBackground() const { return m_DrawTrackBackground; }

 protected:
  int m_NumCores;

  float m_WorldY;
  float m_TextBoxHeight;
  float m_CoresHeight;
  float m_EventTrackHeight;
  float m_GraphTrackHeight;
  float m_TrackBottomMargin;
  float m_TrackLabelOffset;
  float m_SliderWidth;

  float m_SpaceBetweenCores;
  float m_SpaceBetweenCoresAndThread;
  float m_SpaceBetweenTracks;
  float m_SpaceBetweenTracksAndThread;
  float m_SpaceBetweenThreadBlocks;

  float m_TextZ;
  float m_TrackZ;

  bool m_DrawProperties = false;
  bool m_DrawTrackBackground = true;
};

#endif  // ORBIT_GL_TIME_GRAPH_LAYOUT_H_