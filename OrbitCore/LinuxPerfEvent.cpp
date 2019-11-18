#if __linux__

#include "LinuxPerfEvent.h"
#include "LinuxPerfEventVisitor.h"

//-----------------------------------------------------------------------------
void LinuxPerfLostEvent::accept(LinuxPerfEventVisitor* v) { v->visit(this); }

//-----------------------------------------------------------------------------
void LinuxForkEvent::accept(LinuxPerfEventVisitor* v) { v->visit(this); }

//-----------------------------------------------------------------------------
void LinuxSchedSwitchEvent::accept(LinuxPerfEventVisitor* v) { v->visit(this); }

#endif