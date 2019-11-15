#pragma once

#include "LinuxPerfEvent.h"
#include "LinuxPerfEventVisitor.h"

class LinuxEventTracerVisitor : public LinuxPerfEventVisitor
{
public:
    void visit(LinuxPerfLostEvent* e) override;
    void visit(LinuxForkEvent* e) override;
    void visit(LinuxSchedSwitchEvent* e) override;
};