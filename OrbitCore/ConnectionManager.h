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
    void InitAsRemote(std::string a_Host);
    void Stop();

protected:
    void ConnectionThread();
    void TerminateThread();
    void SetupTestMessageHandler();
    void SendTestMessage();

protected:
    std::unique_ptr<std::thread> m_Thread;
    std::string                  m_Host;
    std::atomic<bool>            m_ExitRequested;
};