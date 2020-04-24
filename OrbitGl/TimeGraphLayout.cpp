#include "TimeGraphLayout.h"

#include "Capture.h"
#include "ImGuiOrbit.h"

//-----------------------------------------------------------------------------
TimeGraphLayout::TimeGraphLayout() {
  m_NumCores = 0;
  m_WorldY = 0.f;
  m_TextBoxHeight = 20.f;
  m_CoresHeight = 5.f;
  m_EventTrackHeight = 10.f;
  m_GraphTrackHeight = 20.f;
  m_TrackBottomMargin = 5.f;
  m_SpaceBetweenCores = 2.f;
  m_SpaceBetweenCoresAndThread = 30.f;
  m_SpaceBetweenTracks = 20.f;
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
float TimeGraphLayout::GetCoreOffset(int a_CoreId) const {
  if (Capture::GHasContextSwitches) {
    float coreOffset = m_WorldY - m_CoresHeight -
                       a_CoreId * (m_CoresHeight + m_SpaceBetweenCores);
    return coreOffset;
  }

  return 0.f;
}

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
  FLOAT_SLIDER(m_GraphTrackHeight);
  FLOAT_SLIDER(m_SpaceBetweenCores);
  FLOAT_SLIDER(m_SpaceBetweenCoresAndThread);
  FLOAT_SLIDER(m_SpaceBetweenTracks);
  FLOAT_SLIDER(m_SpaceBetweenTracksAndThread);
  FLOAT_SLIDER(m_SpaceBetweenThreadBlocks);
  FLOAT_SLIDER(m_SliderWidth);
  FLOAT_SLIDER_MIN_MAX(m_TrackBottomMargin, 0, 20.f);
  FLOAT_SLIDER_MIN_MAX(m_TextZ, -1.f, 1.f);
  FLOAT_SLIDER_MIN_MAX(m_TrackZ, -1.f, 1.f);
  ImGui::Checkbox("DrawTrackBackground", &m_DrawTrackBackground);
  ImGui::End();

  return needs_redraw;
}
