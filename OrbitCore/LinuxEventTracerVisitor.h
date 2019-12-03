//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------
#pragma once

#include "LinuxPerfEvent.h"
#include "LinuxPerfEventVisitor.h"
#include "OrbitProcess.h"
#include "Capture.h"

class LinuxEventTracerVisitor : public LinuxPerfEventVisitor
{
public:
    void visit(LinuxPerfLostEvent* a_Event) override;
    void visit(LinuxForkEvent* a_Event) override;
    void visit(LinuxSchedSwitchEvent* a_Event) override;

private:
    std::shared_ptr<Process> m_Process = Capture::GTargetProcess;
};