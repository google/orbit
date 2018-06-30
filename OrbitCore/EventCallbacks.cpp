//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "EventTracer.h"
#include "EventCallbacks.h"
#include "EventTracer.h"
#include "EventClasses.h"
#include "EventUtils.h"
#include "EventGuid.h"

#include "Capture.h"
#include "ContextSwitch.h"
#include "OrbitProcess.h"
#include "SamplingProfiler.h"
#include "TimerManager.h"

#include "evntcons.h"

//-----------------------------------------------------------------------------
std::unordered_map< ULONG64, EventTracing::EventCallback > GEventCallbacks;
std::unordered_map< ULONG64, std::wstring > GFileMap;
std::unordered_map< uint32_t, uint32_t > GThreadToProcessMap;
std::unordered_map< ULONG64, ULONG > GEventCountByProviderId;
bool GOutputEvent = false;

//-----------------------------------------------------------------------------
void EventTracing::Reset();
bool IsTargetProcessThread( uint32_t a_ThreadId );

//-----------------------------------------------------------------------------
inline void ProcessContextSwitch( PEVENT_RECORD a_EventRecord );
inline void ProcessProfileEvent( PEVENT_RECORD a_EventRecord );

//-----------------------------------------------------------------------------
void EventTracing::Init()
{
    if( GEventCallbacks.size() == 0 )
    {
        GEventCallbacks[EventGuid::Hash( ALPCGuid )]            = EventTracing::CallbackALPC;
        GEventCallbacks[EventGuid::Hash( DiskIoGuid )]          = EventTracing::CallbackDiskIo;
        GEventCallbacks[EventGuid::Hash( EventTraceConfigGuid )]= EventTracing::CallbackEventTraceConfig;
        GEventCallbacks[EventGuid::Hash( FileIoGuid )]          = EventTracing::CallbackFileIo;
        GEventCallbacks[EventGuid::Hash( ImageLoadGuid )]       = EventTracing::CallbackImageLoad;
        GEventCallbacks[EventGuid::Hash( PageFaultGuid )]       = EventTracing::CallbackPageFault;
        GEventCallbacks[EventGuid::Hash( PerfInfoGuid )]        = EventTracing::CallbackPerfInfo;
        GEventCallbacks[EventGuid::Hash( ProcessGuid )]         = EventTracing::CallbackProcess;
        GEventCallbacks[EventGuid::Hash( RegistryGuid )]        = EventTracing::CallbackRegistry;
        GEventCallbacks[EventGuid::Hash( SplitIoGuid )]         = EventTracing::CallbackSplitIo;
        GEventCallbacks[EventGuid::Hash( TcpIpGuid )]           = EventTracing::CallbackTcpIp;
        GEventCallbacks[EventGuid::Hash( ThreadGuid )]          = EventTracing::CallbackThread;
        GEventCallbacks[EventGuid::Hash( UdpIpGuid )]           = EventTracing::CallbackUdpIp;
        GEventCallbacks[EventGuid::Hash( StackWalkGuid )]       = EventTracing::CallbackStackWalk;
    }
}

//-----------------------------------------------------------------------------
void EventTracing::Callback( PEVENT_RECORD a_EventRecord )
{
    if( Capture::GTargetProcess && Capture::IsCapturing() )
    {
        ULONG64 idHash = EventGuid::Hash( a_EventRecord->EventHeader.ProviderId );
        ++GEventCountByProviderId[idHash];

        if( EventCallback & callback = GEventCallbacks[idHash] )
        {
            callback( a_EventRecord, a_EventRecord->EventHeader.EventDescriptor.Opcode );
        }
    }
}

//-----------------------------------------------------------------------------
void EventTracing::CallbackALPC( PEVENT_RECORD a_EventRecord, UCHAR a_Opcode )
{
}

//-----------------------------------------------------------------------------
void EventTracing::CallbackDiskIo( PEVENT_RECORD a_EventRecord, UCHAR a_Opcode )
{
    switch( a_Opcode )
    {
    case DiskIo_TypeGroup1::OPCODE_READ:
    case DiskIo_TypeGroup1::OPCODE_WRITE:
    {
        DiskIo_TypeGroup1* event = (DiskIo_TypeGroup1*)a_EventRecord->UserData;
        if( GOutputEvent && IsTargetProcessThread( event->IssuingThreadId ) )
        {
            PRINT_VAR( event->FileObject );
            PRINT_VAR( ws2s( GFileMap[event->FileObject] ) );
            if( GOutputEvent )
            {
                EventUtils::OutputDebugEvent( a_EventRecord );
            }
        }

        break;
    }
    case DiskIo_TypeGroup2::OPCODE_READ_INIT:
    case DiskIo_TypeGroup2::OPCODE_WRITE_INIT:
    case DiskIo_TypeGroup2::OPCODE_FLUSH_INIT:
    {
        DiskIo_TypeGroup2* event = (DiskIo_TypeGroup2*)a_EventRecord->UserData;
        break;
    }

    case DiskIo_TypeGroup3::OPCODE_FLUSH_BUFFER:
        DiskIo_TypeGroup3* event = (DiskIo_TypeGroup3*)a_EventRecord->UserData;
        break;
    }

    if( GOutputEvent )
    {
        EventUtils::OutputDebugEvent(a_EventRecord);
    }
}

//-----------------------------------------------------------------------------
void EventTracing::CallbackEventTraceConfig( PEVENT_RECORD a_EventRecord, UCHAR a_Opcode )
{
}

//-----------------------------------------------------------------------------
void EventTracing::CallbackFileIo( PEVENT_RECORD a_EventRecord, UCHAR a_Opcode )
{
    switch( a_Opcode )
    {
    case FileIo_Name::OPCODE_NAME:
    case FileIo_Name::OPCODE_FILE_CREATE:
    case FileIo_Name::OPCODE_FILE_DELETE:
    case FileIo_Name::OPCODE_FILE_RUNDOWN:
    {
        FileIo_Name* name = (FileIo_Name*)a_EventRecord->UserData;

        std::wstring & fileName = GFileMap[name->FileObject];
        if( fileName != name->FileName )
        {
            GFileMap[name->FileObject] = name->FileName;
        }

        break;
    }

    case FileIo_ReadWrite::OPCODE_READ:
    case FileIo_ReadWrite::OPCODE_WRITE:
    {
        FileIo_ReadWrite* readWrite = (FileIo_ReadWrite*)a_EventRecord->UserData;

        if( GOutputEvent && IsTargetProcessThread( (uint32_t)readWrite->TTID ) )
        {
            PRINT_VAR( readWrite->FileObject );
            PRINT_VAR( ws2s(GFileMap[readWrite->FileObject]) );
        }
        break;
    }
    default:
    {
        static ULONG GUntreated = 0;
        ++GUntreated;
    }
    }
}

//-----------------------------------------------------------------------------
void EventTracing::CallbackImageLoad( PEVENT_RECORD a_EventRecord, UCHAR a_Opcode )
{
}

//-----------------------------------------------------------------------------
void EventTracing::CallbackPageFault( PEVENT_RECORD a_EventRecord, UCHAR a_Opcode )
{
}

//-----------------------------------------------------------------------------
void EventTracing::CallbackPerfInfo( PEVENT_RECORD a_EventRecord, UCHAR a_Opcode )
{
    if( a_Opcode == PerfInfo_SampledProfile::OPCODE )
    {
        ++Capture::GNumProfileEvents;
        if( GEventTracer.IsTracing() )
        {
            ProcessProfileEvent( a_EventRecord );
        }
    }
}

//-----------------------------------------------------------------------------
void EventTracing::CallbackProcess( PEVENT_RECORD a_EventRecord, UCHAR a_Opcode )
{
}

//-----------------------------------------------------------------------------
void EventTracing::CallbackRegistry( PEVENT_RECORD a_EventRecord, UCHAR a_Opcode )
{
}

//-----------------------------------------------------------------------------
void EventTracing::CallbackSplitIo( PEVENT_RECORD a_EventRecord, UCHAR a_Opcode )
{
}

//-----------------------------------------------------------------------------
void EventTracing::CallbackTcpIp( PEVENT_RECORD a_EventRecord, UCHAR a_Opcode )
{
}

//-----------------------------------------------------------------------------
void EventTracing::CallbackThread( PEVENT_RECORD a_EventRecord, UCHAR a_Opcode )
{
    switch( a_Opcode )
    {
        case Thread_TypeGroup1::OPCODE_START:
        case Thread_TypeGroup1::OPCODE_END:
        case Thread_TypeGroup1::OPCODE_DC_START:
        case Thread_TypeGroup1::OPCODE_DC_END:
        {
            Thread_TypeGroup1* event = (Thread_TypeGroup1*)a_EventRecord->UserData;

            //if( event->ProcessId == Capture::GTargetProcess->GetID() )
            {
                // Thread started
                if( a_Opcode == Thread_TypeGroup1::OPCODE_START || a_Opcode == Thread_TypeGroup1::OPCODE_DC_START )
                {
                    GThreadToProcessMap[event->TThreadId] = event->ProcessId;
                }
                else
                {
                    GThreadToProcessMap.erase( event->TThreadId );
                }
            }
        
            break;
        }
        case CSwitch::OPCODE:
        {
            ProcessContextSwitch( a_EventRecord );
        }
    }
}

//-----------------------------------------------------------------------------
void EventTracing::CallbackUdpIp( PEVENT_RECORD a_EventRecord, UCHAR a_Opcode )
{
}

//-----------------------------------------------------------------------------
void EventTracing::CallbackStackWalk( PEVENT_RECORD a_EventRecord, UCHAR a_Opcode )
{
    if( a_Opcode == StackWalk_Event::OPCODE_STACK )
    {
        StackWalk_Event* event = (StackWalk_Event*)a_EventRecord->UserData;

        if( event->StackProcess == Capture::GTargetProcess->GetID() )
        {
            int stackDepth = ( a_EventRecord->UserDataLength - (sizeof(StackWalk_Event) - sizeof(event->Stack1)) )/sizeof(ULONG64);
            
            CallStack CS;
            CS.m_Depth = stackDepth;
            CS.m_ThreadId = event->StackThread;
            size_t numBytes = std::min(stackDepth, ORBIT_STACK_SIZE)*sizeof(ULONG64);
            CS.m_Data.resize(stackDepth);
            memcpy(CS.m_Data.data(), &event->Stack1, numBytes );

            Capture::GSamplingProfiler->AddCallStack( CS );
            GEventTracer.GetEventBuffer().AddCallstackEvent( a_EventRecord->EventHeader.TimeStamp.QuadPart, CS );
        }
    }
}

//-----------------------------------------------------------------------------
void EventTracing::Reset()
{
    Init();
    GThreadToProcessMap.clear();
}

//-----------------------------------------------------------------------------
inline bool IsTargetProcessThread( uint32_t a_ThreadId )
{
    return GThreadToProcessMap[a_ThreadId] == Capture::GTargetProcess->GetID();
}

//-----------------------------------------------------------------------------
inline void ProcessContextSwitch( PEVENT_RECORD a_EventRecord )
{
    EVENT_HEADER &Header = a_EventRecord->EventHeader;
    UCHAR ProcessorNumber = a_EventRecord->BufferContext.ProcessorNumber;
    USHORT ProcessorIndex = a_EventRecord->BufferContext.ProcessorIndex;
    ULONG ThreadID = Header.ThreadId;
    LONGLONG CycleTime = Header.TimeStamp.QuadPart;
    CSwitch* switchEvent = (CSwitch*)a_EventRecord->UserData;

    ++Capture::GNumContextSwitches;
    DWORD processId = Capture::GTargetProcess->GetID();
    if( GThreadToProcessMap[switchEvent->NewThreadId] == processId )
    {
        ContextSwitch CS( ContextSwitch::In );
        CS.m_ThreadId = switchEvent->NewThreadId;
        CS.m_Time = CycleTime;
        CS.m_ProcessorIndex = ProcessorIndex;
        CS.m_ProcessorNumber = ProcessorNumber;
        GTimerManager->Add( CS );
    }

    if( GThreadToProcessMap[switchEvent->OldThreadId] == processId )
    {
        ContextSwitch CS( ContextSwitch::Out );
        CS.m_ThreadId = switchEvent->OldThreadId;
        CS.m_Time = CycleTime;
        CS.m_ProcessorIndex = ProcessorIndex;
        CS.m_ProcessorNumber = ProcessorNumber;
        GTimerManager->Add( CS );
    }
}

//-----------------------------------------------------------------------------
inline void ProcessProfileEvent( PEVENT_RECORD a_EventRecord )
{
    EVENT_HEADER &Header = a_EventRecord->EventHeader;
    UCHAR ProcessorNumber = a_EventRecord->BufferContext.ProcessorNumber;
    USHORT ProcessorIndex = a_EventRecord->BufferContext.ProcessorIndex;
    ULONG ThreadID = Header.ThreadId;
    LONGLONG CycleTime = Header.TimeStamp.QuadPart;
    PerfInfo_SampledProfile* sampleEvent = (PerfInfo_SampledProfile*)a_EventRecord->UserData;

    if( Capture::GTargetProcess->HasThread( sampleEvent->ThreadId ) && Capture::IsCapturing() )
    {
        /*CallStack CS;
        CS.m_ThreadId = ThreadID;
        CS.m_Data[CS.m_Depth++] = sampleEvent->InstructionPointer;
        Capture::GSamplingProfiler->AddCallStack( CS );*/
    }
}
