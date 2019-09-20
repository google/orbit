//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>

class ConnectionManager
{
public:
    ConnectionManager();
    ~ConnectionManager();
    static ConnectionManager& Get();
    void Init();
    void InitAsRemote();
    void ConnectToRemote(std::string a_RemoteAddress);
    bool IsRemote(){ return m_IsRemote; } // True when Orbit instance is headless, next to remote process
    void StartCaptureAsRemote();
    void StopCaptureAsRemote();
    void Stop();

protected:
    void ConnectionThread();
    void RemoteThread();
    void TerminateThread();
    void SetupClientCallbacks();
    void SetupServerCallbacks();
    void SendProcesses(class TcpEntity* a_TcpEntity);

protected:
    std::unique_ptr<std::thread> m_Thread;
    std::string                  m_RemoteAddress;
    std::atomic<bool>            m_ExitRequested;
    bool                         m_IsRemote;
};