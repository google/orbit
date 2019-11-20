#pragma once

#include "LinuxPerfEvent.h"

// TODO: As the class hierachy is not fixed yet, we need to 
//  keep this in sync.
class LinuxPerfEventVisitor
{
public:
    virtual void visit(LinuxPerfLostEvent* a_Event) = 0;
    virtual void visit(LinuxForkEvent* a_Event) = 0;
    virtual void visit(LinuxSchedSwitchEvent* a_Event) = 0;
};