//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ConnectionManager.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "TimerManager.h"
#include "Serialization.h"

#include <fstream>

//-----------------------------------------------------------------------------
struct CrossPlatformMessage
{
    CrossPlatformMessage();

    void Fill();
    void Save();
    void Load();
    void Dump();

    ORBIT_SERIALIZABLE;
    std::string m_Name;
    std::vector<std::string> m_Strings;
    std::map<uint32_t, std::string> m_StringMap;
    std::string m_SerializationPath;
};

//-----------------------------------------------------------------------------
CrossPlatformMessage::CrossPlatformMessage()
{
    m_SerializationPath = ws2s( Path::GetBasePath() ) + "xmsg.orbit";
    PRINT_VAR(m_SerializationPath);
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE( CrossPlatformMessage, 1 )
{
    ORBIT_NVP_VAL( 0, m_Name );
    ORBIT_NVP_VAL( 0, m_Strings );
    ORBIT_NVP_VAL( 0, m_StringMap );
}

//-----------------------------------------------------------------------------
void CrossPlatformMessage::Fill()
{
    m_Name = "Orbit CrossPlatformMessage";
    for( int i = 0 ; i < 1000; ++i )
    {
        std::string val = std::to_string(i);
        m_Strings.push_back(val);
        m_StringMap[i] = val;
    }
}

//-----------------------------------------------------------------------------
void CrossPlatformMessage::Save()
{
    std::ofstream myfile( m_SerializationPath, std::ios::binary );
    if( !myfile.fail() )
    {
        SCOPE_TIMER_LOG( Format( L"Saving cross platform message" ) );
        cereal::BinaryOutputArchive archive( myfile );
        archive(*this);
        myfile.close();
    }
}

//-----------------------------------------------------------------------------
void CrossPlatformMessage::Load()
{
    std::ifstream myfile( m_SerializationPath, std::ios::binary );
    if( !myfile.fail() )
    {
        cereal::BinaryInputArchive archive( myfile );
        archive(*this);
        myfile.close();
    }

    Dump();
}

//-----------------------------------------------------------------------------
void CrossPlatformMessage::Dump()
{
    PRINT_VAR(m_Name);
    for( std::string& str : m_Strings )
    {
        PRINT_VAR(str);
    }

    for( auto pair : m_StringMap )
    {
        PRINT_VAR(pair.first);
        PRINT_VAR(pair.second);
    }
}

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

    //CrossPlatformMessage xmsg;
    //xmsg.Fill();
    //xmsg.Load();
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
