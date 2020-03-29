//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once
#include <map>
#include <unordered_map>
#include <vector>

#include "CallstackTypes.h"
#include "CoreMath.h"
#include "ThreadTrackMap.h"

//-----------------------------------------------------------------------------
class TimeGraphLayout {
 public:
  TimeGraphLayout();

  float GetCoreOffset(int a_CoreId);
  float GetThreadStart();
  float GetThreadBlockStart(ThreadID a_TID);
  float GetThreadOffset(ThreadID a_TID, int a_Depth = 0);
  float GetTracksHeight();
  float GetSamplingTrackOffset(ThreadID a_TID);
  float GetFileIOTrackOffset(ThreadID a_TID);
  bool IsThreadVisible(ThreadID a_TID);
  float GetTotalHeight();
  float GetTextBoxHeight() const { return m_TextBoxHeight; }
  float GetTextCoresHeight() const { return m_CoresHeight; }
  float GetEventTrackHeight() const { return m_EventTrackHeight; }
  float GetTrackBottomMargin() const { return m_TrackBottomMargin; }
  float GetTrackLabelOffset() const { return m_TrackLabelOffset; }
  float GetSliderWidth() const { return m_SliderWidth; }
  float GetSpaceBetweenTracksAndThread() const {
    return m_SpaceBetweenTracksAndThread;
  }
  float GetTextZ() const { return m_TextZ; }
  float GetTrackZ() const { return m_TrackZ; }
  void CalculateOffsets(const ThreadTrackMap& a_ThreadTracks);
  void Reset();
  void SetDrawProperties(bool value) { m_DrawProperties = value; }

  void SetSortedThreadIds(const std::vector<ThreadID>& a_SortedThreadIds);
  const std::vector<ThreadID>& GetSortedThreadIds() const {
    return m_SortedThreadIds;
  }

  void SetNumCores(int a_NumCores) { m_NumCores = a_NumCores; }
  bool DrawProperties();

 protected:
  void SortTracksByPosition(const ThreadTrackMap& a_ThreadTracks);

 protected:
  int m_NumCores;
  int m_NumTracksPerThread;

  bool m_DrawFileIO;

  float m_WorldY;

  float m_TextBoxHeight;
  float m_CoresHeight;
  float m_EventTrackHeight;
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

  int m_NumTracks;

  std::map<ThreadID, int> m_ThreadDepths;
  std::map<ThreadID, float> m_ThreadBlockOffsets;
  std::vector<ThreadID> m_SortedThreadIds;

  ThreadTrackMap* m_ThreadTrackMap;
  bool m_DrawProperties = false;
};
