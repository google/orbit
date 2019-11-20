#pragma once

#include "LinuxPerfEvent.h"
#include "LinuxPerfEventVisitor.h"
#include "OrbitProcess.h"
#include "Capture.h"

class LinuxEventTracerVisitor : public LinuxPerfEventVisitor
{
public:
    void visit(LinuxPerfLostEvent* e) override;
    void visit(LinuxForkEvent* e) override;
    void visit(LinuxSchedSwitchEvent* e) override;

private:
    std::shared_ptr<Process> m_Process = Capture::GTargetProcess;
};