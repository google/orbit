//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ConnectionManager.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "TimerManager.h"

//-----------------------------------------------------------------------------
ConnectionManager::ConnectionManager() : m_ExitRequested(false)
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
void ConnectionManager::Init(std::string a_Host)
{
    m_Host = a_Host;
    TerminateThread();
    m_Thread = std::make_unique<std::thread>(&ConnectionManager::ConnectionThread, this);
}

//-----------------------------------------------------------------------------
void ConnectionManager::Stop()
{
    m_ExitRequested = true;
}

//-----------------------------------------------------------------------------
void ConnectionManager::ConnectionThread()
{
    while (!m_ExitRequested)
    {
        if (!GTcpClient || !GTcpClient->IsValid())
        {
            GTcpClient = std::make_unique<TcpClient>(m_Host);
            if (GTcpClient->IsValid())
            {
                GTimerManager = std::make_unique<TimerManager>(true);
            }
        }
        else
        {
            std::string msg("Hello from ConnectionManager");
            GTcpClient->Send(msg);
            Sleep(2000);
        }
    }
}
