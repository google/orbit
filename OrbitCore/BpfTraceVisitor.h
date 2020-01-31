#pragma once

#include "LinuxPerfEvent.h"
#include "LinuxPerfEventVisitor.h"
#include "ScopeTimer.h"

class BpfTraceVisitor : public LinuxPerfEventVisitor {
 public:
  void visit(LinuxPerfLostEvent* a_Event) override;
  void visit(LinuxUprobeEvent* a_Event) override;
  void visit(LinuxUprobeEventWithStack* a_Event) override;
  void visit(LinuxUretprobeEvent* a_Event) override;
  void visit(LinuxUretprobeEventWithStack* a_Event) override;

 private:
  std::map<uint64_t, std::vector<Timer>> m_TimerStacks;
};