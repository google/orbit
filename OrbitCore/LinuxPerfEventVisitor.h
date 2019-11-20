#pragma once

#include "LinuxPerfEvent.h"

// TODO: As the class hierarchy of LinuxPerfEvent is not fixed yet, we need to 
//  keep this class in sync wtih the hierarchy.
class LinuxPerfEventVisitor
{
public:
    virtual void visit(LinuxPerfLostEvent* a_Event) = 0;
    virtual void visit(LinuxForkEvent* a_Event) = 0;
    virtual void visit(LinuxSchedSwitchEvent* a_Event) = 0;
};