#ifndef ORBIT_GL_SCHEDULER_TRACK_H_
#define ORBIT_GL_SCHEDULER_TRACK_H_

#include "ThreadTrack.h"

class SchedulerTrack : public ThreadTrack {
 public:
  SchedulerTrack(TimeGraph* time_graph);
  ~SchedulerTrack() override = default;

  void Draw(GlCanvas* a_Canvas, bool a_Picking) override;
  void UpdatePrimitives(uint64_t min_tick, uint64_t max_tick) override;
  Type GetType() const override { return kSchedulerTrack; }
  float GetHeight() const override;

  protected:
  float GetYFromDepth(float track_y, uint32_t depth, bool collapsed) override;
};

#endif  // ORBIT_GL_SCHEDULER_TRACK_H_
