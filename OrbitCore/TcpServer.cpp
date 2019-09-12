//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Core.h"
#include "TcpServer.h"
#include "Tcp.h"
#include "VariableTracing.h"
#include "Log.h"
#include "Context.h"
#include "Capture.h"
#include "OrbitAsio.h"
#include "Callstack.h"
#include "SamplingProfiler.h"
#include "TimerManager.h"
#include "OrbitUnreal.h"
#include "OrbitProcess.h"

#include <thread>

TcpServer* GTcpServer;

//-----------------------------------------------------------------------------
TcpServer::TcpServer() : m_TcpServer(nullptr)
{
    m_LastNumMessages = 0;
    m_LastNumBytes = 0;
    m_NumReceivedMessages = 0;
    m_NumMessagesPerSecond = 0;
    m_BytesPerSecond = 0;
    m_MaxTimersAtOnce = 0;
    m_NumTimersAtOnce = 0;
    m_NumTargetQueuedEntries = 0;
    m_NumTargetFlushedEntries = 0;
    m_NumTargetFlushedTcpPackets = 0;
    m_NumMessagesFromPreviousSession = 0;
}

//-----------------------------------------------------------------------------
TcpServer::~TcpServer()
{
    delete m_TcpServer;
}

//-----------------------------------------------------------------------------
void TcpServer::Start( unsigned short a_Port )
{
    TcpEntity::Start();

    PRINT_FUNC;
    m_TcpService = new TcpService();
    m_TcpServer = new tcp_server( *m_TcpService->m_IoService, a_Port );

    PRINT_VAR(a_Port);

    std::thread t([&](){ this->ServerThread(); });
    t.detach();

    m_StatTimer.Start();
}

//-----------------------------------------------------------------------------
void TcpServer::ResetStats()
{
    m_NumReceivedMessages = 0;
    if(m_TcpServer)
        m_TcpServer->ResetStats();
}

//-----------------------------------------------------------------------------
std::vector<std::string> TcpServer::GetStats()
{
    std::vector<std::string> stats;
    stats.push_back( VAR_TO_ANSI( m_NumReceivedMessages ) );
    stats.push_back( VAR_TO_ANSI( m_NumMessagesPerSecond ) );

    std::string bytesRcv = "Capture::GNumBytesReceiced = " + ws2s( GetPrettySize( m_TcpServer->GetNumBytesReceived() ) ) + "\n";
        stats.push_back( bytesRcv );
        std::string bitRate = "Capture::Bitrate = "
            + ws2s( GetPrettySize( (ULONG64)m_BytesPerSecond ) )
            + "/s"
            + " ( " + GetPrettyBitRate( (ULONG64)m_BytesPerSecond ) + " )\n";
    
    stats.push_back( bitRate );
    return stats;
}

//-----------------------------------------------------------------------------
TcpSocket* TcpServer::GetSocket()
{
    return m_TcpServer->GetSocket();
}

//-----------------------------------------------------------------------------
void TcpServer::Receive( const Message & a_Message )
{
    const Message::Header & MessageHeader = a_Message.GetHeader();
    ++m_NumReceivedMessages;

    // Disregard messages from previous session
    if( a_Message.m_SessionID != Message::GSessionID )
    {
        ++m_NumMessagesFromPreviousSession;
        return;
    }

    switch (a_Message.GetType())
    {
    case Msg_String:
    {
        const char* msg = a_Message.GetData();
        std::cout << msg << std::endl;
        PRINT_VAR(msg);
        break;
    }
    case Msg_Timer:
    {
        uint32_t numTimers = (uint32_t)a_Message.m_Size/sizeof(Timer);
        Timer* timers = (Timer*)a_Message.GetData();
        for (uint32_t i = 0; i < numTimers; ++i)
        {
            GTimerManager->Add(timers[i]);
        }
        
        if( numTimers > m_MaxTimersAtOnce )
        {
            m_MaxTimersAtOnce = numTimers;
        }
        m_NumTimersAtOnce = numTimers;

        break;
    }
    case Msg_NumQueuedEntries:
        m_NumTargetQueuedEntries = *((uint32_t*)a_Message.GetData());
        break;
	case Msg_NumFlushedEntries:
		m_NumTargetFlushedEntries = *((uint32_t*)a_Message.GetData());
		break;
    case Msg_NumFlushedItems:
        m_NumTargetFlushedTcpPackets = *( (uint32_t*)a_Message.GetData() );
        break;
    case Msg_NumInstalledHooks:
        Capture::GNumInstalledHooks = *((uint32_t*)a_Message.GetData());
        break;
    case Msg_Callstack:
    {
        CallStackPOD* callstackPOD = (CallStackPOD*)a_Message.GetData();
        CallStack callstack(*callstackPOD);
        Capture::AddCallstack( callstack );
        break;
    }
    case Msg_OrbitZoneName:
    {
        OrbitZoneName* zoneName = (OrbitZoneName*)a_Message.GetData();
        Capture::RegisterZoneName( zoneName->m_Address, zoneName->m_Data );
        break;
    }
    case Msg_OrbitUnrealObject:
    {
        const UnrealObjectHeader & header = MessageHeader.m_UnrealObjectHeader;
        
        std::wstring & objectName = GOrbitUnreal.GetObjectNames()[header.m_Ptr];
        if( header.m_WideStr )
        {
            objectName = (wchar_t*)a_Message.GetData();
        }
        else
        {
            objectName = s2ws( (char*)a_Message.GetData() );
        }
        
        break;
    }
    case Msg_ThreadInfo:
    {
        std::wstring threadName((wchar_t*)a_Message.GetData());
        Capture::GTargetProcess->SetThreadName(a_Message.m_ThreadId, threadName);
        PRINT_VAR(threadName);
        break;
    }
    default:
    {
        Callback(a_Message);
        break;
    }
    }
}

//-----------------------------------------------------------------------------
void TcpServer::SendToUiAsync( const std::wstring & a_Message )
{
    if( m_UiCallback )
    {
        m_UiLockFreeQueue.enqueue( a_Message );
    }
}

//-----------------------------------------------------------------------------
void TcpServer::SendToUiNow( const std::wstring & a_Message )
{
    if( m_UiCallback )
    {
        m_UiCallback( a_Message );
    }
}

//-----------------------------------------------------------------------------
void TcpServer::MainThreadTick()
{
    std::wstring msg;
    while( m_UiLockFreeQueue.try_dequeue( msg ) )
    {
        m_UiCallback( msg );
    }

    static double period = 500.0;
    double elapsedTime = m_StatTimer.QueryMillis();
    if( elapsedTime > period )
    {
        m_NumMessagesPerSecond = ((double)(m_NumReceivedMessages - m_LastNumMessages))/(elapsedTime*0.001);
        m_LastNumMessages = m_NumReceivedMessages;

        uint64_t numBytesReceived = m_TcpServer ? m_TcpServer->GetNumBytesReceived() : 0;
        m_BytesPerSecond = (double( numBytesReceived - m_LastNumBytes))/(elapsedTime*0.001);
        m_LastNumBytes = numBytesReceived;
        m_StatTimer.Reset();
    }

    if( Capture::GInjected && Capture::IsCapturing() )
    {
        TcpSocket* socket = GetSocket();
        if( socket == nullptr || !socket->m_Socket || !socket->m_Socket->is_open() )
        {
            Capture::StopCapture();
        }
    }
}

//-----------------------------------------------------------------------------
bool TcpServer::IsLocalConnection()
{
    TcpSocket* socket = GetSocket();
    if( socket != nullptr && socket->m_Socket )
    {
        std::string endPoint = socket->m_Socket->remote_endpoint().address().to_string();
        if( endPoint == "127.0.0.1" || ToLower( endPoint ) == "localhost" )
        {
            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------------
void TcpServer::Disconnect()
{ 
    PRINT_FUNC;
    m_TcpServer->Disconnect(); 
}

//-----------------------------------------------------------------------------
bool TcpServer::HasConnection()
{ 
    return m_TcpServer->HasConnection();
}

//-----------------------------------------------------------------------------
void TcpServer::ServerThread()
{
    PRINT_FUNC;
    SetCurrentThreadName(L"TcpServer");
    m_TcpService->m_IoService->run();
}
