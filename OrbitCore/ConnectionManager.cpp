//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ConnectionManager.h"
#include "Capture.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "TimerManager.h"
#include "Serialization.h"
#include "TestRemoteMessages.h"
#include "EventBuffer.h"
#include "ProcessUtils.h"
#include "LinuxPerf.h"
#include "SamplingProfiler.h"
#include "CoreApp.h"
#include "OrbitModule.h"
#include "BpfTrace.h"

#if __linux__
#include "LinuxUtils.h"
#endif

#include <fstream>
#include <streambuf>
#include <sstream>
#include <iostream>

//-----------------------------------------------------------------------------
ConnectionManager::ConnectionManager() : m_ExitRequested(false), m_IsRemote(false)
{
    m_BpfTrace = std::make_shared<BpfTrace>();
}

//-----------------------------------------------------------------------------
ConnectionManager::~ConnectionManager()
{
    TerminateThread();
}

//-----------------------------------------------------------------------------
void ConnectionManager::TerminateThread()
{
    if (m_Thread)
    {
        m_ExitRequested = true;
        m_Thread->join();
        m_Thread = nullptr;
    }
}

//-----------------------------------------------------------------------------
ConnectionManager& ConnectionManager::Get()
{
    static ConnectionManager instance;
    return instance;
}

//-----------------------------------------------------------------------------
void ConnectionManager::ConnectToRemote(std::string a_RemoteAddress)
{
    m_RemoteAddress = a_RemoteAddress;
    TerminateThread();
    SetupClientCallbacks();
    m_Thread = std::make_unique<std::thread>(&ConnectionManager::ConnectionThread, this);
}

//-----------------------------------------------------------------------------
void ConnectionManager::InitAsRemote()
{
    m_IsRemote = true;
    SetupServerCallbacks();
    m_Thread = std::make_unique<std::thread>(&ConnectionManager::RemoteThread, this);
}

//-----------------------------------------------------------------------------
void ConnectionManager::StartCaptureAsRemote()
{
    PRINT_FUNC;
    Capture::StartCapture();
    m_BpfTrace->Start();
}

//-----------------------------------------------------------------------------
void ConnectionManager::StopCaptureAsRemote()
{
    PRINT_FUNC;
    m_BpfTrace->Stop();
    Capture::StopCapture();
}

//-----------------------------------------------------------------------------
void ConnectionManager::Stop()
{
    m_ExitRequested = true;
}

//-----------------------------------------------------------------------------
void ConnectionManager::SetupServerCallbacks()
{
    GTcpServer->AddMainThreadCallback( Msg_BpfScript, [=](const Message& a_Msg )
    {
        m_BpfTrace->SetBpfScript(a_Msg.GetData());
    } );

    GTcpServer->AddMainThreadCallback( Msg_StartCapture, [=]( const Message & a_Msg )
    {
        StartCaptureAsRemote();
    } );

    GTcpServer->AddMainThreadCallback( Msg_StopCapture, [=]( const Message & a_Msg )
    {
        StopCaptureAsRemote();
    } );

    GTcpServer->AddMainThreadCallback( Msg_RemoteProcessRequest, [=](const Message & a_Msg)
    {
        uint32_t pid = (uint32_t)a_Msg.m_Header.m_GenericHeader.m_Address;
        GCoreApp->SendRemoteProcess(pid);
    });

    
    GTcpServer->AddMainThreadCallback( Msg_RemoteModuleDebugInfo, [=](const Message & a_Msg)
    {
        std::vector<ModuleDebugInfo> remoteModuleDebugInfo;
        std::vector<std::string> modules;

        std::istringstream buffer(std::string(a_Msg.m_Data, a_Msg.m_Size));
        cereal::JSONInputArchive inputAr(buffer);
        inputAr(modules);

        for (std::string& module : modules)
        {
            ModuleDebugInfo moduleDebugInfo;
            moduleDebugInfo.m_Name = module;
            Capture::GTargetProcess->FillModuleDebugInfo(moduleDebugInfo);
            remoteModuleDebugInfo.push_back(moduleDebugInfo);
        }

        // Send data back
        std::string messageData = SerializeObjectHumanReadable(remoteModuleDebugInfo);
        GTcpServer->Send(Msg_RemoteModuleDebugInfo, (void*)messageData.data(), messageData.size());
        
    });
}

//-----------------------------------------------------------------------------
void ConnectionManager::SetupClientCallbacks()
{
    GTcpClient->AddMainThreadCallback( Msg_RemotePerf, [=]( const Message & a_Msg )
    {
        PRINT_VAR(a_Msg.m_Size);
        std::string msgStr(a_Msg.m_Data, a_Msg.m_Size);
        std::istringstream buffer(msgStr);
        LinuxPerf perf(0);

        Capture::NewSamplingProfiler();
        Capture::GSamplingProfiler->StartCapture();
        Capture::GSamplingProfiler->SetIsLinuxPerf(true);
        perf.LoadPerfData(buffer);
        Capture::GSamplingProfiler->StopCapture();
        Capture::GSamplingProfiler->ProcessSamples();
        GCoreApp->RefreshCaptureView();
    } );

    
    GTcpClient->AddCallback(Msg_RemoteTimers, [=](const Message& a_Msg)
    {
        uint32_t numTimers = (uint32_t)a_Msg.m_Size / sizeof(Timer);
        Timer* timers = (Timer*)a_Msg.GetData();
        for (uint32_t i = 0; i < numTimers; ++i)
        {
            GTimerManager->Add(timers[i]);
        }
    } );

    GTcpClient->AddCallback(Msg_RemoteCallStack, [=](const Message& a_Msg)
    {
        CallStack stack;
        std::istringstream buffer(std::string(a_Msg.m_Data, a_Msg.m_Size));
        cereal::JSONInputArchive inputAr(buffer);
        inputAr(stack);

        GCoreApp->ProcessCallStack(&stack);
    } );
}

//-----------------------------------------------------------------------------
void ConnectionManager::SendProcesses(TcpEntity* a_TcpEntity)
{
    ProcessList processList;
    processList.Refresh();
    processList.UpdateCpuTimes();
    std::string processData = SerializeObjectHumanReadable(processList);
    a_TcpEntity->Send(Msg_RemoteProcessList, (void*)processData.data(), processData.size());
}

//-----------------------------------------------------------------------------
void ConnectionManager::ConnectionThread()
{
    while (!m_ExitRequested)
    {
        if (!GTcpClient->IsValid())
        {
            GTcpClient->Connect( m_RemoteAddress );
            GTcpClient->Start();
        }
        else
        {
            //std::string msg("Hello from dev machine");
            //GTcpClient->Send(msg);
        }

        Sleep(2000);
    }
}

//-----------------------------------------------------------------------------
void ConnectionManager::RemoteThread()
{
    while (!m_ExitRequested)
    {
        //PRINT_VAR(GTcpServer->HasConnection());
        //if (GTcpServer->HasConnection())
        {
            std::string msg("Hello from remote instance");
            GTcpServer->Send(msg);
            SendProcesses(GTcpServer);
        }

        Sleep(2000);
    }
}
