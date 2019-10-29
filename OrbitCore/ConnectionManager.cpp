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
#include "OrbitFunction.h"
#include "BpfTrace.h"
#include "Params.h"

#if __linux__
#include "LinuxUtils.h"
#endif

#include <map>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <iostream>
#include <cereal/types/vector.hpp>

//-----------------------------------------------------------------------------
ConnectionManager::ConnectionManager() : m_ExitRequested(false), m_IsRemote(false)
{

    if (GParams.m_UseBPFTrace)
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
void ConnectionManager::SetSelectedFunctionsOnRemote( const char* a_Data, size_t a_Size ) {
    PRINT_FUNC;
    std::istringstream buffer(std::string(a_Data, a_Size));
    cereal::JSONInputArchive inputAr( buffer );
    std::vector<std::string> selectedFunctions;
    inputAr(selectedFunctions);

    Capture::GSelectedFunctionsMap.clear();
    for (Function* function : Capture::GTargetProcess->GetFunctions()) {
        function->UnSelect();
    }

    for (const std::string& address : selectedFunctions) {
        PRINT_VAR(address);
        Function* function = Capture::GTargetProcess->GetFunctionFromAddress(std::stoll(address));
        if (!function)
            PRINT("received invalid address");
        else{
            PRINT(Format("Received Selected Function: %s\n", function->m_PrettyName.c_str()));
            // this also adds the function to the map.
            function->Select();
        }

    }
}

//-----------------------------------------------------------------------------
void ConnectionManager::StartCaptureAsRemote()
{
    PRINT_FUNC;
    Capture::StartCapture();
    if (GParams.m_UseBPFTrace)
        m_BpfTrace->Start();
}

//-----------------------------------------------------------------------------
void ConnectionManager::StopCaptureAsRemote()
{
    PRINT_FUNC;
    if (GParams.m_UseBPFTrace)
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
    if (GParams.m_UseBPFTrace)
    {
        GTcpServer->AddMainThreadCallback( Msg_BpfScript, [=](const Message& a_Msg )
        {
            m_BpfTrace->SetBpfScript(a_Msg.GetData());
        } );
    }

    GTcpServer->AddMainThreadCallback( Msg_RemoteSelectedFunctionsMap, [=]( const Message & a_Msg )
    {
        SetSelectedFunctionsOnRemote(a_Msg.GetData(), a_Msg.m_Size);
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
    GTcpClient->AddMainThreadCallback( Msg_RemotePerfSampling, [=]( const Message & a_Msg )
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

        GTcpClient->AddMainThreadCallback( Msg_RemotePerfUProbes, [=]( const Message & a_Msg )
    {
        PRINT_VAR(a_Msg.m_Size);
        std::string msgStr(a_Msg.m_Data, a_Msg.m_Size);
        std::istringstream buffer(msgStr);
        LinuxPerf perf(0);

        perf.LoadPerfData(buffer);
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
