#ifndef ORBIT_GL_SCHEDULER_TRACK_H_
#define ORBIT_GL_SCHEDULER_TRACK_H_

#include "ThreadTrack.h"

class SchedulerTrack : public ThreadTrack {
 public:
  SchedulerTrack(TimeGraph* time_graph, uint32_t thread_id)
      : ThreadTrack(time_graph, thread_id) {}
  ~SchedulerTrack() override = default;

  void Draw(GlCanvas* a_Canvas, bool a_Picking) override;
  Type GetType() const override { return kSchedulerTrack; }
  float GetHeight() const override;
};

#endif  // ORBIT_GL_SCHEDULER_TRACK_H_