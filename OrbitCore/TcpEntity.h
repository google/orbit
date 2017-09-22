//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Message.h"
#include "TcpForward.h"
#include "Threading.h"
#include "../OrbitPlugin/OrbitUserData.h"

#include <type_traits>
#include <vector>
#include <atomic>

//-----------------------------------------------------------------------------
class TcpPacket
{
public:
    TcpPacket(){}
    explicit TcpPacket( const Message & a_Message
                      , const void* a_Payload )
                      : m_Data( new std::vector<char>() )
    {
        m_Data->resize( sizeof( Message ) + a_Message.m_Size + 4 );
        memcpy( m_Data->data(), &a_Message, sizeof( Message ) );

        if( a_Payload )
        {
            memcpy( m_Data->data() + sizeof( Message ), a_Payload, a_Message.m_Size );
        }

        // Footer
        const unsigned int footer = MAGIC_FOOT_MSG;
        memcpy( m_Data->data() + sizeof( Message ) + a_Message.m_Size, &footer, 4 );
    }

    std::shared_ptr< std::vector<char> > Data() { return m_Data; };

private:
    std::shared_ptr< std::vector<char> > m_Data;
};

//-----------------------------------------------------------------------------
class TcpEntity
{
public:
    TcpEntity();
    ~TcpEntity();

    virtual void Start();
    void Stop();
    void FlushSendQueue();

    // Note: All Send methods can be called concurrently from multiple threads
    inline void Send(MessageType a_Type) { SendMsg(Message(a_Type), nullptr); }
    inline void Send(Message & a_Message, void* a_Data);
    inline void Send(Message & a_Message);
    inline void Send(std::string& a_String);
    inline void Send(OrbitLogEntry& a_Entry);
    inline void Send(Orbit::UserData& a_Entry);
    inline void Send( MessageType a_Type , void* a_Data, size_t a_Size );

    template<class T> void Send( Message & a_Message, const std::vector<T> & a_Vector );
    template<class T> void Send( MessageType a_Type , const std::vector<T> & a_Vector );
    template<class T> void Send( Message & a_Message, const T& a_Item );
    template<class T> void Send( MessageType a_Type , const T& a_Item );

protected:
    void SendMsg( Message & a_Message, const void* a_Payload );
    virtual TcpSocket* GetSocket() = 0;
    void SendData();

protected:
    TcpService*                m_TcpService;
    TcpSocket*                 m_TcpSocket;
    std::thread*               m_SenderThread;
    AutoResetEvent             m_ConditionVariable;
    LockFreeQueue< TcpPacket > m_SendQueue;
    std::atomic<int>           m_NumQueuedEntries;
    std::atomic<bool>          m_ExitRequested = false;
    std::atomic<bool>          m_FlushRequested = false;
    std::atomic<int>           m_NumFlushedItems = 0;
};

//-----------------------------------------------------------------------------
void TcpEntity::Send( Message & a_Message, void* a_Data )
{
    SendMsg( a_Message, a_Data );
}

//-----------------------------------------------------------------------------
inline void TcpEntity::Send( Message & a_Message )
{
    SendMsg( a_Message, a_Message.m_Data );
}

//-----------------------------------------------------------------------------
void TcpEntity::Send( std::string& a_String )
{
    Message msg(Msg_String, (int)(a_String.size() + 1)*sizeof(a_String[0]));
    Send(msg, (void*)a_String.data());
}

//-----------------------------------------------------------------------------
void TcpEntity::Send( OrbitLogEntry& a_Entry )
{
    char stackBuffer[1024];
    int entrySize = (int)a_Entry.GetBufferSize();
    bool needsAlloc = entrySize > 1024;
    char* buffer = !needsAlloc ? stackBuffer : new char[entrySize];
    
    memcpy( buffer, &a_Entry, a_Entry.GetSizeWithoutString() );
    memcpy( buffer + a_Entry.GetSizeWithoutString(), a_Entry.m_Text.c_str(), a_Entry.GetStringSize() );

    Message msg( Msg_OrbitLog, entrySize );
    Send( msg, (void*)buffer );

    if( needsAlloc )
    {
        delete buffer;
    }
}

//-----------------------------------------------------------------------------
void TcpEntity::Send( Orbit::UserData& a_UserData )
{
    Message msg( Msg_UserData );
    msg.m_Size = sizeof(Orbit::UserData) + a_UserData.m_NumBytes;
    
    char stackBuffer[1024];
    bool needsAlloc = msg.m_Size > 1024;
    char* buffer = !needsAlloc ? stackBuffer : new char[msg.m_Size];

    memcpy( buffer, &a_UserData, sizeof(Orbit::UserData) );
    memcpy( buffer + sizeof(Orbit::UserData), a_UserData.m_Data, a_UserData.m_NumBytes );

    Send( msg, (void*)buffer );

    if( needsAlloc )
    {
        delete buffer;
    }
}

//-----------------------------------------------------------------------------
template<class T> void TcpEntity::Send( Message & a_Message, const std::vector<T> & a_Vector )
{
    a_Message.m_Size = (int)a_Vector.size()*sizeof(T);
    SendMsg(a_Message, (void*)a_Vector.data());
}

//-----------------------------------------------------------------------------
template<class T> void TcpEntity::Send( MessageType a_Type, const std::vector<T> & a_Vector )
{
    Send(Message(a_Type), a_Vector);
}

//-----------------------------------------------------------------------------
template<class T> void TcpEntity::Send( Message & a_Message, const T& a_Item )
{
    a_Message.m_Size = (int)sizeof(T);
    SendMsg(a_Message, (void*)&a_Item);
}

//-----------------------------------------------------------------------------
template<class T> void TcpEntity::Send( MessageType a_Type, const T& a_Item )
{
    Send( Message(a_Type), a_Item );
}

//-----------------------------------------------------------------------------
void TcpEntity::Send( MessageType a_Type , void* a_Data, size_t a_Size )
{
    Message a_Message(a_Type);
    a_Message.m_Size = (int)a_Size;
    SendMsg( a_Message, a_Data );
}


