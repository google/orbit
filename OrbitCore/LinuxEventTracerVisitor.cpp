#include "OrbitProcess.h"
#include "ContextSwitch.h"
#include "TimerManager.h"
#include "LinuxEventTracerVisitor.h"
#include "LinuxPerfEvent.h"
#include "PrintVar.h"

void LinuxEventTracerVisitor::visit(LinuxPerfLostEvent* e)
{
    PRINT("Lost %u Events\n", e->Lost());
}

void LinuxEventTracerVisitor::visit(LinuxForkEvent* e)
{
    if (m_Process->HasThread(e->ParentTID()))
    {
        m_Process->AddThreadId(e->TID());
    }
}

void LinuxEventTracerVisitor::visit(LinuxSchedSwitchEvent* e)
{
    // the known thread stopped running
    if (m_Process->HasThread(e->PrevPID()))
    {
        ++Capture::GNumContextSwitches;

        ContextSwitch CS ( ContextSwitch::Out );
        CS.m_ThreadId = e->PrevPID();
        CS.m_Time = e->Timestamp();
        CS.m_ProcessorIndex = e->CPU();
        //TODO: Is this correct?
        CS.m_ProcessorNumber = e->CPU();
        GTimerManager->Add( CS );
    }

    // the known thread starts running
    if (m_Process->HasThread(e->NextPID()))
    {
        ++Capture::GNumContextSwitches;

        ContextSwitch CS ( ContextSwitch::In );
        CS.m_ThreadId = e->NextPID();
        CS.m_Time = e->Timestamp();
        CS.m_ProcessorIndex = e->CPU();
        //TODO: Is this correct?
        CS.m_ProcessorNumber = e->CPU();
        GTimerManager->Add( CS );
    }
}