//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------

#include "LinuxPerfEvent.h"

#include "LinuxPerfEventVisitor.h"

// These cannot be implemented in the header LinuxPerfEvent.h, because there
// LinuxPerfEventVisitor needs to be an incomplete type to avoid the circular
// dependency between LinuxPerfEvent.h and LinuxPerfEventVisitor.h.

void LinuxPerfLostEvent::accept(LinuxPerfEventVisitor* visitor) {
  visitor->visit(this);
}

void LinuxForkEvent::accept(LinuxPerfEventVisitor* visitor) {
  visitor->visit(this);
}

void LinuxExitEvent::accept(LinuxPerfEventVisitor* visitor) {
  visitor->visit(this);
}

void LinuxContextSwitchEvent::accept(LinuxPerfEventVisitor* visitor) {
  visitor->visit(this);
}

void LinuxSystemWideContextSwitchEvent::accept(LinuxPerfEventVisitor* visitor) {
  visitor->visit(this);
}

void LinuxStackSampleEvent::accept(LinuxPerfEventVisitor* visitor) {
  visitor->visit(this);
}

void LinuxUprobeEvent::accept(LinuxPerfEventVisitor* visitor) {
  visitor->visit(this);
}

void LinuxUprobeEventWithStack::accept(LinuxPerfEventVisitor* visitor) {
  visitor->visit(this);
}

void LinuxUretprobeEvent::accept(LinuxPerfEventVisitor* visitor) {
  visitor->visit(this);
}

void LinuxUretprobeEventWithStack::accept(LinuxPerfEventVisitor* visitor) {
  visitor->visit(this);
}

void LinuxMapsEvent::accept(LinuxPerfEventVisitor* visitor) {
  visitor->visit(this);
}
