//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#ifdef _WIN32
//#define _WIN32_WINNT 0x0501
#include <sdkddkver.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <ctime>
#include <iostream>
#include <string>
#include <unordered_set>
#include <asio.hpp>

#include "Message.h"
#include "TcpEntity.h"
#include "OrbitAsio.h"

using asio::ip::tcp;

//-----------------------------------------------------------------------------
class TcpConnection : public std::enable_shared_from_this < TcpConnection >
{
public:
    typedef std::shared_ptr<TcpConnection> pointer;

    ~TcpConnection();

    static pointer create( asio::io_service& io_service )
    {
        return pointer( new TcpConnection( io_service ) );
    }

    TcpSocket& GetSocket()
    {
        return m_WrappedSocket;
    }

    void start()
    {
        ReadMessage();
    }

    void ReadMessage();
    void ReadPayload();
	void ReadFooter();
    void DecodeMessage( Message & a_Message );

    bool IsWebsocket() { return m_WebSocketKey != ""; }
    void ReadWebsocketHandshake();
    void ReadWebsocketMessage();
    void ReadWebsocketMask();
    void ReadWebsocketPayload();
    void DecodeWebsocketPayload();
    ULONG64 GetNumBytesReceived(){ return m_NumBytesReceived; }

    void ResetStats();
    std::vector<std::string> GetStats();

private:
    TcpConnection( asio::io_service& io_service )
        : m_Socket( io_service )
        , m_WrappedSocket( &m_Socket )
    {
        m_NumBytesReceived = 0;
    }
    // handle_write() is responsible for any further actions 
    // for this client connection.
    void handle_write( const asio::error_code& /*error*/,
        size_t /*bytes_transferred*/ )
    {
    }

    void handle_request_line( asio::error_code ec, std::size_t bytes_transferred );
    void SendWebsocketResponse();

    tcp::socket         m_Socket;
    TcpSocket           m_WrappedSocket;
    Message             m_Message;
    std::vector<char>   m_Payload;
    asio::streambuf     m_StreamBuf;
    std::string         m_WebSocketKey;
    char                m_WebSocketBuffer[MAX_WS_HEADER_LENGTH];
    unsigned int        m_WebSocketPayloadLength;
    unsigned int        m_WebSocketMask;
    ULONG64             m_NumBytesReceived;
};

//-----------------------------------------------------------------------------
class tcp_server : public std::enable_shared_from_this < tcp_server >
{
public:
    tcp_server( asio::io_service & io_service, unsigned short port );
    ~tcp_server();

    void Disconnect();
    bool HasConnection(){ return m_Connection != nullptr; }
    TcpSocket* GetSocket(){ return m_Connection ? &m_Connection->GetSocket() : nullptr; }
    void RegisterConnection( std::shared_ptr<TcpConnection> a_Connection );
    ULONG64 GetNumBytesReceived(){ return m_Connection ? m_Connection->GetNumBytesReceived() : 0; }
    void ResetStats(){ if( m_Connection ) m_Connection->ResetStats(); }

private:
    void start_accept();
    void handle_accept( TcpConnection::pointer new_connection, const asio::error_code& error );

    tcp::acceptor m_Acceptor;
    std::shared_ptr<TcpConnection> m_Connection;
    std::unordered_set< std::shared_ptr<TcpConnection> > m_ConnectionsSet;
};

//-----------------------------------------------------------------------------
class shared_const_buffer
{
public:
    shared_const_buffer(){}
    explicit shared_const_buffer( const Message & a_Message
                                , const void* a_Payload )
                                : data_(new std::vector<char>() )
    {
        data_->resize( sizeof(Message) + a_Message.m_Size + 4 );
        memcpy( data_->data(), &a_Message, sizeof(Message) );

        if (a_Payload)
        {
            memcpy(data_->data()+sizeof(Message), a_Payload, a_Message.m_Size);
        }

		// Footer
		const unsigned int footer = MAGIC_FOOT_MSG;
		memcpy(data_->data() + sizeof(Message) + a_Message.m_Size, &footer, 4);

        buffer_ = asio::buffer(*data_);
    }

    explicit shared_const_buffer( TcpPacket & a_Packet )
    {
        data_ = a_Packet.Data();
        buffer_ = asio::buffer( *data_ );
    }

    // Implement the ConstBufferSequence requirements.
    typedef asio::const_buffer value_type;
    typedef const asio::const_buffer* const_iterator;
    const asio::const_buffer* begin() const { return &buffer_; }
    const asio::const_buffer* end() const { return &buffer_ + 1; }

    std::shared_ptr< std::vector<char> > Data() { return data_; };

private:
    std::shared_ptr< std::vector<char> > data_;
    asio::const_buffer buffer_;
};

