#include "PerfEvent.h"

#include "PerfEventVisitor.h"

namespace LinuxTracing {

// These cannot be implemented in the header PerfEvent.h, because there
// PerfEventVisitor needs to be an incomplete type to avoid the circular
// dependency between PerfEvent.h and PerfEventVisitor.h.

void LostPerfEvent::accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void ForkPerfEvent::accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void ExitPerfEvent::accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void ContextSwitchPerfEvent::accept(PerfEventVisitor* visitor) {
  visitor->visit(this);
}

void SystemWideContextSwitchPerfEvent::accept(PerfEventVisitor* visitor) {
  visitor->visit(this);
}

void StackSamplePerfEvent::accept(PerfEventVisitor* visitor) {
  visitor->visit(this);
}

void UprobePerfEvent::accept(PerfEventVisitor* visitor) {
  visitor->visit(this);
}

void UprobePerfEventWithStack::accept(PerfEventVisitor* visitor) {
  visitor->visit(this);
}

void UretprobePerfEvent::accept(PerfEventVisitor* visitor) {
  visitor->visit(this);
}

void UretprobePerfEventWithStack::accept(PerfEventVisitor* visitor) {
  visitor->visit(this);
}

void MapsPerfEvent::accept(PerfEventVisitor* visitor) { visitor->visit(this); }

}  // namespace LinuxTracing
