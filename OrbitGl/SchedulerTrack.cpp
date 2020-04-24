#include "SchedulerTrack.h"
#include "TimeGraph.h"

void SchedulerTrack::Draw(GlCanvas* /*a_Canvas*/, bool /*a_Picking*/) {
  // Do nothing.
}

float SchedulerTrack::GetHeight() const {
  TimeGraphLayout& layout = time_graph_->GetLayout();
  float depth = static_cast<float>(m_Depth);
  return (layout.GetTextCoresHeight() + layout.GetSpaceBetweenCores()) * depth +
         layout.GetTrackBottomMargin();
}
