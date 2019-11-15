#include "OrbitProcess.h"
#include "ContextSwitch.h"
#include "TimerManager.h"
#include "Capture.h"
#include "LinuxEventTracerVisitor.h"
#include "LinuxPerfEvent.h"
#include "PrintVar.h"

void LinuxEventTracerVisitor::visit(LinuxPerfLostEvent* e)
{
    PRINT("Lost %u Events", e->Lost());
}

void LinuxEventTracerVisitor::visit(LinuxForkEvent* e)
{
    // todo: move this as a class member
    auto targetProcess = Capture::GTargetProcess;
    // todo: check if this is correct:
    // todo: it might be also correct to check check if ppid == process.id
    if (targetProcess->HasThread(e->ParentTID()))
    {
        targetProcess->AddThreadId(e->TID());
        if (e->ParentPID() != targetProcess->GetID())
        {
            PRINT("ASD\n");
        }
    }
}

void LinuxEventTracerVisitor::visit(LinuxSchedSwitchEvent* e)
{
    // todo: move this as a class member
    auto targetProcess = Capture::GTargetProcess;

    // the known thread stopped running
    if (targetProcess->HasThread(e->PrevPID()))
    {
        ++Capture::GNumContextSwitches;

        ContextSwitch CS ( ContextSwitch::Out );
        CS.m_ThreadId = e->PrevPID();
        CS.m_Time = e->Timestamp();
        CS.m_ProcessorIndex = e->CPU();
        //TODO: Is this correct?
        CS.m_ProcessorNumber = e->CPU();
        GTimerManager->Add( CS );
        PRINT("OUT%u\n", e->PrevPID());
    }

    // the known thread starts running
    if (targetProcess->HasThread(e->NextPID()))
    {
        ++Capture::GNumContextSwitches;

        ContextSwitch CS ( ContextSwitch::In );
        CS.m_ThreadId = e->NextPID();
        CS.m_Time = e->Timestamp();
        CS.m_ProcessorIndex = e->CPU();
        //TODO: Is this correct?
        CS.m_ProcessorNumber = e->CPU();
        GTimerManager->Add( CS );

        PRINT("IN%u\n", e->NextPID());
    }
}