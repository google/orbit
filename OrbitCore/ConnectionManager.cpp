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

#include <fstream>
#include <streambuf>
#include <sstream>
#include <iostream>

//-----------------------------------------------------------------------------
struct CrossPlatformMessage
{
    CrossPlatformMessage();

    void Fill();
    void Save();
    void Load();
    void Dump();

    std::string ToRaw();
    void FromRaw(const char* data, uint32_t size);

    ORBIT_SERIALIZABLE;
    std::wstring m_WName;
    uint32_t m_Id;
    std::string m_Name;
    std::vector<std::string> m_Strings;
    std::map<uint32_t, std::string> m_StringMap;
    std::string m_SerializationPath;
    uint32_t m_Size;
    uint32_t m_SizeOnSave;
};

//-----------------------------------------------------------------------------
CrossPlatformMessage::CrossPlatformMessage()
{
    static uint32_t id = 0;
    m_Id = ++id;
    m_Size = sizeof(CrossPlatformMessage);
    m_SizeOnSave = 0;
    m_SerializationPath = ws2s( Path::GetBasePath() ) + "xmsg.orbit";
    PRINT_VAR(m_SerializationPath);
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE( CrossPlatformMessage, 1 )
{
    ORBIT_NVP_VAL( 0, m_Name );
    ORBIT_NVP_VAL( 0, m_Strings );
    ORBIT_NVP_VAL( 0, m_StringMap );
    ORBIT_NVP_VAL( 1, m_SizeOnSave );
    ORBIT_NVP_VAL( 1, m_Id );
    //ORBIT_NVP_VAL( 1, m_WName );
}

//-----------------------------------------------------------------------------
void CrossPlatformMessage::Fill()
{
    m_Name = "Orbit CrossPlatformMessage";
    m_WName = L"Wname";
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
        m_SizeOnSave = sizeof(CrossPlatformMessage);
        SCOPE_TIMER_LOG( Format( L"Saving cross platform message" ) );
        cereal::BinaryOutputArchive archive( myfile );
        archive(*this);
        myfile.close();
    }
}

//-----------------------------------------------------------------------------
std::string CrossPlatformMessage::ToRaw()
{
    m_SizeOnSave = sizeof(CrossPlatformMessage);
    std::stringstream buffer;
    cereal::BinaryOutputArchive archive( buffer );
    archive(*this);
    PRINT_VAR(buffer.str().size());
    return buffer.str();
}

void CrossPlatformMessage::FromRaw(const char* data, uint32_t size)
{
    PRINT_FUNC;
    PRINT_VAR(size);
    std::istringstream buffer(std::string(data, size));
    cereal::BinaryInputArchive inputAr( buffer );
    inputAr(*this);
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
void ConnectionManager::Init()
{
    SetupTestMessageHandler();
}

//-----------------------------------------------------------------------------
void ConnectionManager::InitAsRemote(std::string a_Host)
{
    m_Host = a_Host;
    TerminateThread();
    m_Thread = std::make_unique<std::thread>(&ConnectionManager::ConnectionThread, this);

    //CrossPlatformMessage xmsg;
    //xmsg.Fill();
    //xmsg.Load();
}

//-----------------------------------------------------------------------------
void ConnectionManager::StartCaptureAsRemote()
{
    Capture::StartCapture();
}

//-----------------------------------------------------------------------------
void ConnectionManager::StopCaptureAsRemote()
{
    Capture::StopCapture();
}

//-----------------------------------------------------------------------------
void ConnectionManager::Stop()
{
    m_ExitRequested = true;
}

//-----------------------------------------------------------------------------
void ConnectionManager::SetupTestMessageHandler()
{
    GTcpServer->AddCallback( Msg_CrossPlatform, [=]( const Message & a_Msg )
    {
        CrossPlatformMessage msg;
        PRINT_VAR(a_Msg.m_Size);
        msg.FromRaw(a_Msg.m_Data, a_Msg.m_Size);
        msg.Dump();
    } );
}

//-----------------------------------------------------------------------------
void ConnectionManager::SetupClientCallbacks()
{
    GTcpClient->AddCallback( Msg_StartCapture, [=]( const Message & a_Msg )
    {
        StartCaptureAsRemote();
    } );

    GTcpClient->AddCallback( Msg_StopCapture, [=]( const Message & a_Msg )
    {
        StopCaptureAsRemote();
    } );
}

//-----------------------------------------------------------------------------
void ConnectionManager::SendTestMessage()
{
    CrossPlatformMessage msg;
    msg.Fill();

    std::string rawMsg = msg.ToRaw();
    GTcpClient->Send(Msg_CrossPlatform, (void*)rawMsg.data(), rawMsg.size());

    //TestRemoteMessages::Get().Run();
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
                SetupClientCallbacks();
                GTimerManager = std::make_unique<TimerManager>(true);
            }
        }
        else
        {
            std::string msg("Hello from ConnectionManager");
            GTcpClient->Send(msg);

            SendTestMessage();

            Sleep(2000);
        }
    }
}
