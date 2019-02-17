//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Core.h"
#include "TcpEntity.h"
#include <functional>
#include <unordered_map>

class TcpServer : public TcpEntity
{
public:
    TcpServer();
    ~TcpServer();
    
    void Start( unsigned short a_Port );

    void Receive( const Message & a_Message );

    void SendToUiAsync( const std::wstring & a_Message );
    void SendToUiNow( const std::wstring & a_Message );
    void SendToUiAsync( const std::string & a_Message ){ SendToUiAsync(s2ws(a_Message)); }
    void SendToUiNow( const std::string & a_Message ){ SendToUiNow(s2ws(a_Message)); }

    typedef std::function< void( const Message & ) > MsgCallback;
    typedef std::function< void( const std::wstring & ) > StrCallback;

    void SetCallback( MessageType a_MsgType, MsgCallback a_Callback ) { m_Callbacks[a_MsgType] = a_Callback; }
    void SetUiCallback( StrCallback a_Callback ){ m_UiCallback = a_Callback; }
    void MainThreadTick();

    void Disconnect();
    bool HasConnection();
    
    bool IsLocalConnection();

    class tcp_server* GetServer(){ return m_TcpServer; }

    void ResetStats();
    std::vector< std::string > GetStats();

protected:
    class TcpSocket* GetSocket() override final;
    void ServerThread();

private:
    class tcp_server*                         m_TcpServer;
    std::unordered_map< int, MsgCallback >    m_Callbacks;
    StrCallback                               m_UiCallback;
    moodycamel::ConcurrentQueue<std::wstring> m_UiLockFreeQueue;
    
    Timer   m_StatTimer;
    ULONG64 m_LastNumMessages;
    ULONG64 m_LastNumBytes;
    ULONG64 m_NumReceivedMessages;
    double  m_NumMessagesPerSecond;
    double  m_BytesPerSecond;
    int     m_MaxTimersAtOnce;
    int     m_NumTimersAtOnce;
    int     m_NumTargetQueuedEntries;
    int     m_NumTargetFlushedEntries;
    int     m_NumTargetFlushedTcpPackets;
    ULONG64 m_NumMessagesFromPreviousSession;
};

extern TcpServer* GTcpServer;

