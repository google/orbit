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

//-----------------------------------------------------------------------------
void LinuxPerfLostEvent::accept(LinuxPerfEventVisitor* a_Visitor) { a_Visitor->visit(this); }

//-----------------------------------------------------------------------------
void LinuxForkEvent::accept(LinuxPerfEventVisitor* a_Visitor) { a_Visitor->visit(this); }

//-----------------------------------------------------------------------------
void LinuxExitEvent::accept(LinuxPerfEventVisitor* a_Visitor) { a_Visitor->visit(this); }

//-----------------------------------------------------------------------------
void LinuxContextSwitchEvent::accept(LinuxPerfEventVisitor* a_Visitor) { a_Visitor->visit(this); }

//-----------------------------------------------------------------------------
void LinuxSystemWideContextSwitchEvent::accept(LinuxPerfEventVisitor* a_Visitor) { a_Visitor->visit(this); }

//-----------------------------------------------------------------------------
void LinuxStackSampleEvent::accept(LinuxPerfEventVisitor* a_Visitor) { a_Visitor->visit(this); }

//-----------------------------------------------------------------------------
void LinuxUprobeEvent::accept(LinuxPerfEventVisitor* a_Visitor) { a_Visitor->visit(this); }

//-----------------------------------------------------------------------------
void LinuxUprobeEventWithStack::accept(LinuxPerfEventVisitor* a_Visitor) { a_Visitor->visit(this); }

//-----------------------------------------------------------------------------
void LinuxUretprobeEvent::accept(LinuxPerfEventVisitor* a_Visitor) { a_Visitor->visit(this); }