#include "TimeGraphLayout.h"

#include "Capture.h"
#include "ImGuiOrbit.h"
#include "Params.h"
#include "ThreadTrack.h"

//-----------------------------------------------------------------------------
TimeGraphLayout::TimeGraphLayout() {
  m_NumCores = std::thread::hardware_concurrency();

  Reset();

  m_WorldY = 0.f;
  m_TextBoxHeight = 20.f;
  m_CoresHeight = 5.f;
  m_EventTrackHeight = 10.f;
  m_TrackBottomMargin = 5.f;
  m_SpaceBetweenCores = 2.f;
  m_SpaceBetweenCoresAndThread = 30.f;
  m_SpaceBetweenTracks = 2.f;
  m_SpaceBetweenTracksAndThread = 5.f;
  m_SpaceBetweenThreadBlocks = 35.f;
  m_TrackLabelOffset = 6.f;
  m_SliderWidth = 15.f;
  m_TextZ = -0.02f;
  m_TrackZ = -0.1f;
};

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetThreadStart() {
  if (Capture::GHasContextSwitches) {
    return m_WorldY - m_NumCores * m_CoresHeight -
           std::max(m_NumCores - 1, 0) * m_SpaceBetweenCores -
           m_SpaceBetweenCoresAndThread;
  }

  return m_WorldY;
}

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetTracksHeight() {
  return m_NumTracks ? m_NumTracks * m_EventTrackHeight +
                           std::max(m_NumTracks - 1, 0) * m_SpaceBetweenTracks +
                           m_SpaceBetweenTracksAndThread
                     : 0;
}

//-----------------------------------------------------------------------------
void TimeGraphLayout::SortTracksByPosition(
    const ThreadTrackMap& a_ThreadTracks) {
  std::vector<std::shared_ptr<ThreadTrack> > tracks;

  for (auto& pair : a_ThreadTracks) {
    if (pair.second->GetVisible()) {
      tracks.push_back(pair.second);
    }
  }

  std::sort(tracks.begin(), tracks.end(),
            [](const std::shared_ptr<ThreadTrack>& a,
               const std::shared_ptr<ThreadTrack>& b) -> bool {
              return a->GetPos()[1] > b->GetPos()[1];
            });

  // Reorder m_SortedThreadIds.
  std::vector<ThreadID> sortedThreadIds;
  for (auto& track : tracks) {
    ThreadID tid = track->GetID();
    if (std::find(m_SortedThreadIds.begin(), m_SortedThreadIds.end(), tid) !=
        m_SortedThreadIds.end()) {
      sortedThreadIds.push_back(tid);
    }
  }

  m_SortedThreadIds = sortedThreadIds;
}

//-----------------------------------------------------------------------------
void TimeGraphLayout::CalculateOffsets(const ThreadTrackMap& a_ThreadTracks) {
  m_ThreadBlockOffsets.clear();

  m_NumTracks = 1;
  if (m_DrawFileIO) ++m_NumTracks;

  // TODO: Fix SortTrackByPosition messing up the normal thread sorting.
  if (false && !Capture::IsCapturing()) {
    SortTracksByPosition(a_ThreadTracks);
  }

  float offset = GetThreadStart();
  for (ThreadID threadID : m_SortedThreadIds) {
    auto iter = a_ThreadTracks.find(threadID);
    if (iter == a_ThreadTracks.end()) continue;

    std::shared_ptr<ThreadTrack> track = iter->second;
    m_ThreadBlockOffsets[threadID] = offset;
    float threadBlockHeight =
        GetTracksHeight() + track->GetDepth() * m_TextBoxHeight;
    offset -= (threadBlockHeight + m_SpaceBetweenThreadBlocks);
  }

  for (auto& pair : a_ThreadTracks) {
    auto& track = pair.second;
    if (track.get() && track->IsMoving()) {
      m_ThreadBlockOffsets[track->GetID()] = track->GetPos()[1];
    }
  }
}

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetCoreOffset(int a_CoreId) {
  if (Capture::GHasContextSwitches) {
    float coreOffset = m_WorldY - m_CoresHeight -
                       a_CoreId * (m_CoresHeight + m_SpaceBetweenCores);
    return coreOffset;
  }

  return 0.f;
}

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetThreadOffset(ThreadID a_TID, int a_Depth) {
  return m_ThreadBlockOffsets[a_TID] - GetTracksHeight() -
         (a_Depth + 1) * m_TextBoxHeight;
}

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetThreadBlockStart(ThreadID a_TID) {
  return m_ThreadBlockOffsets[a_TID];
}

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetSamplingTrackOffset(ThreadID a_TID) {
  auto iter = m_ThreadBlockOffsets.find(a_TID);
  if (iter != m_ThreadBlockOffsets.end()) return m_ThreadBlockOffsets[a_TID];
  return -1.f;
}

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetFileIOTrackOffset(ThreadID a_TID) {
  return m_DrawFileIO ? GetSamplingTrackOffset(a_TID) - m_SpaceBetweenTracks -
                            m_EventTrackHeight
                      : 0.f;
}

//-----------------------------------------------------------------------------
bool TimeGraphLayout::IsThreadVisible(ThreadID a_TID) {
  return std::find(m_SortedThreadIds.begin(), m_SortedThreadIds.end(), a_TID) !=
         m_SortedThreadIds.end();
}

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetTotalHeight() {
  if (m_SortedThreadIds.size() > 0) {
    ThreadID threadId = m_SortedThreadIds.back();
    float offset = GetThreadOffset(threadId, m_ThreadDepths[threadId]);
    return std::abs(offset - m_WorldY) + m_TextBoxHeight;
  }

  return GetThreadStart();
}

//-----------------------------------------------------------------------------
void TimeGraphLayout::SetSortedThreadIds(
    const std::vector<ThreadID>& a_SortedThreadIds) {
  m_SortedThreadIds = a_SortedThreadIds;
}

//-----------------------------------------------------------------------------
void TimeGraphLayout::Reset() { m_DrawFileIO = false; }

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
  FLOAT_SLIDER(m_TrackLabelOffset);
  FLOAT_SLIDER(m_TextBoxHeight);
  FLOAT_SLIDER(m_CoresHeight);
  FLOAT_SLIDER(m_EventTrackHeight);
  FLOAT_SLIDER(m_SpaceBetweenCores);
  FLOAT_SLIDER(m_SpaceBetweenCoresAndThread);
  FLOAT_SLIDER(m_SpaceBetweenTracks);
  FLOAT_SLIDER(m_SpaceBetweenTracksAndThread);
  FLOAT_SLIDER(m_SpaceBetweenThreadBlocks);
  FLOAT_SLIDER(m_SliderWidth);
  FLOAT_SLIDER_MIN_MAX(m_TrackBottomMargin, 0, 20.f);
  FLOAT_SLIDER_MIN_MAX(m_TextZ, -1.f, 1.f);
  FLOAT_SLIDER_MIN_MAX(m_TrackZ, -1.f, 1.f);
  ImGui::End();

  return needs_redraw;
}
