#include "SchedulerTrack.h"

#include "TimeGraph.h"

void SchedulerTrack::Draw(GlCanvas* canvas, bool picking) {
  ThreadTrack::Draw(canvas, picking);
}

float SchedulerTrack::GetHeight() const {
  TimeGraphLayout& layout = time_graph_->GetLayout();
  float depth = static_cast<float>(depth_);
  return (layout.GetTextCoresHeight() + layout.GetSpaceBetweenCores()) * depth +
         layout.GetTrackBottomMargin();
}
