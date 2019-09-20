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
}

//-----------------------------------------------------------------------------
void ConnectionManager::StopCaptureAsRemote()
{
    PRINT_FUNC;
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
    PRINT_FUNC;
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
    GTcpClient->AddCallback( Msg_RemotePerf, [=]( const Message & a_Msg )
    {
        PRINT_VAR(a_Msg.m_Size);
        std::istringstream buffer(std::string(a_Msg.m_Data, a_Msg.m_Size));
        LinuxPerf perf(0);

        Capture::NewSamplingProfiler();
        Capture::GSamplingProfiler->StartCapture();
        Capture::GSamplingProfiler->SetIsLinuxPerf(true);
        perf.LoadPerfData(buffer);
        Capture::GSamplingProfiler->StopCapture();
        Capture::GSamplingProfiler->ProcessSamples();

    } );
}

//-----------------------------------------------------------------------------
void ConnectionManager::SendProcesses(TcpEntity* a_TcpEntity)
{
    ProcessList processList;
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
        //if (GTcpServer->HasConnection())F
        {
            std::string msg("Hello from remote instance");
            GTcpServer->Send(msg);
            SendProcesses(GTcpServer);
        }

        Sleep(2000);
    }
}
