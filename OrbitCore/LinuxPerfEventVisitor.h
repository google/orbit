//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------
#pragma once

#include "LinuxPerfEvent.h"

// TODO: As the class hierarchy of LinuxPerfEvent is not fixed yet, we need to 
//  keep this class in sync wtih the hierarchy.
class LinuxPerfEventVisitor
{
public:
    virtual void visit(LinuxPerfLostEvent* a_Event) {};
    virtual void visit(LinuxForkEvent* a_Event) {};
    virtual void visit(LinuxContextSwitchEvent* a_Event) {};
    virtual void visit(LinuxSystemWideContextSwitchEvent* a_Event) {};
    virtual void visit(LinuxUprobeEvent* a_Event) {};
    virtual void visit(LinuxUprobeEventWithStack* a_Event) {};
    virtual void visit(LinuxUretprobeEvent* a_Event) {};
};