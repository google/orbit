//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Tcp.h"
#include "Core.h"
#include "TcpServer.h"
#include "Capture.h"
#include "PrintVar.h"

/*
#include "websocketpp/frame.hpp"
#include "websocketpp/base64/base64.hpp"
#include "websocketpp/sha1/sha1.hpp"
*/

// Based off asio sample
// https://github.com/chriskohlhoff/asio/

//-----------------------------------------------------------------------------
tcp_server::~tcp_server()
{
    PRINT_FUNC;
}

//-----------------------------------------------------------------------------
tcp_server::tcp_server( asio::io_service & io_service, unsigned short port )
    : m_Acceptor( io_service, tcp::endpoint( tcp::v4(), port ) )
{
    start_accept();
}

//-----------------------------------------------------------------------------
void tcp_server::Disconnect()
{
    PRINT_FUNC;
    if( m_Connection )
    {
        Message msg( Msg_Unload, 0, nullptr );
        GTcpServer->Send( msg );
        m_Connection = nullptr;
    }
}

//-----------------------------------------------------------------------------
void tcp_server::RegisterConnection( std::shared_ptr<TcpConnection> a_Connection )
{
    m_Connection = a_Connection;
}

//-----------------------------------------------------------------------------
void tcp_server::start_accept()
{
    // creates a socket
    TcpConnection::pointer new_connection = TcpConnection::create( m_Acceptor.get_io_service() );

    // initiates an asynchronous accept operation 
    // to wait for a new connection. 
    m_Acceptor.async_accept( *new_connection->GetSocket().m_Socket
                           , std::bind( &tcp_server::handle_accept
                                      , this
                                      , new_connection
                                      , std::placeholders::_1 ) 
                           );
}

// handle_accept() is called when the asynchronous accept operation 
// initiated by start_accept() finishes. It services the client request
//-----------------------------------------------------------------------------
void tcp_server::handle_accept(TcpConnection::pointer new_connection, const asio::error_code& error)
{
    if( !error )
    {
        PRINT_FUNC;
        m_ConnectionsSet.insert( new_connection );
        new_connection->start();
    }

    start_accept();
}

//-----------------------------------------------------------------------------
TcpConnection::~TcpConnection()
{
    PRINT_VAR( "TcpConnection::~TcpConnection()" );
}

//-----------------------------------------------------------------------------
void TcpConnection::ReadMessage()
{
    asio::async_read( m_Socket, asio::buffer( &m_Message, sizeof(Message) ), 
        
    [this]( asio::error_code ec, std::size_t /*length*/ )
    {
        if(!ec)
        {
            m_NumBytesReceived += sizeof(Message);
            ReadPayload();
        }
        else
        {
            PRINT_VAR( ec.message() );
            m_Socket.close();
        }
    }
    
    );
}

//-----------------------------------------------------------------------------
void TcpConnection::ReadWebsocketMessage()
{
    /*asio::async_read( m_Socket, asio::buffer( &m_WebSocketBuffer, sizeof(websocketpp::frame::basic_header) ),

        [this]( asio::error_code ec, std::size_t  )
    {
        if( !ec )
        {
            websocketpp::frame::basic_header& basicHeader = *reinterpret_cast<websocketpp::frame::basic_header*>( &m_WebSocketBuffer );
            m_WebSocketPayloadLength = get_basic_size( basicHeader);
            assert( m_WebSocketPayloadLength < 126 ); // Big messages not supported yet
            assert( get_masked( basicHeader ) );
            ReadWebsocketMask();
        }
        else
        {
            PRINT_VAR( ec.message() );
            m_Socket.close();
        }
    }

    );*/
}

//-----------------------------------------------------------------------------
void TcpConnection::ReadWebsocketMask()
{
    asio::async_read( m_Socket, asio::buffer( &m_WebSocketMask, sizeof( m_WebSocketMask ) ),

        [this]( asio::error_code ec, std::size_t /*length*/ )
    {
        if( !ec )
        {
            ReadWebsocketPayload();
        }
        else
        {
            PRINT_VAR( ec.message() );
            m_Socket.close();
        }
    }

    );
}

//-----------------------------------------------------------------------------
void TcpConnection::ReadWebsocketPayload()
{
    m_Payload.resize( m_WebSocketPayloadLength + 1);
    m_Payload[m_WebSocketPayloadLength] = 0;
    asio::async_read( m_Socket, asio::buffer( m_Payload.data(), m_WebSocketPayloadLength ),

        [this]( asio::error_code ec, std::size_t /*length*/ )
    {
        if( !ec )
        {
            DecodeWebsocketPayload();
        }
        else
        {
            PRINT_VAR( ec.message() );
            m_Socket.close();
        }
    }

    );
}

//-----------------------------------------------------------------------------
void TcpConnection::DecodeWebsocketPayload()
{
    uint8_t* mask = (uint8_t*)&m_WebSocketMask;
    for( unsigned int i = 0; i < m_WebSocketPayloadLength; ++i )
    {
        m_Payload[i] = m_Payload[i] ^ mask[i%4];
    }

    std::string cmd = m_Payload.data();
    PRINT_VAR(cmd);
    GTcpServer->SendToUiAsync( cmd );
    ReadWebsocketMessage();
}

//-----------------------------------------------------------------------------
void TcpConnection::ResetStats()
{
    m_NumBytesReceived = 0;
}

//-----------------------------------------------------------------------------
std::vector<std::string> TcpConnection::GetStats()
{
    return std::vector<std::string>();
}

//-----------------------------------------------------------------------------
bool IsWebSocketHandshakeMessage( Message& a_Message )
{
    char* a_String = reinterpret_cast<char*>( &a_Message );
    return a_String[0] == 'G' &&
           a_String[1] == 'E' &&
           a_String[2] == 'T';
}

//-----------------------------------------------------------------------------
void process_handshake_key( std::string & key )
{
    /*static char const ws_handshake_guid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    key.append( ws_handshake_guid );

    unsigned char message_digest[20];
    websocketpp::sha1::calc( key.c_str(), key.length(), message_digest );
    key = websocketpp::base64_encode( message_digest, 20 );*/
}

//-----------------------------------------------------------------------------
void TcpConnection::handle_request_line( asio::error_code ec, std::size_t bytes_transferred )
{
    if( !ec )
    {
        m_NumBytesReceived += bytes_transferred;

        asio::streambuf::const_buffers_type bufs = m_StreamBuf.data();
        std::string str( asio::buffers_begin( bufs ), asio::buffers_begin( bufs ) + bytes_transferred );

        if( Contains( str, "Sec-WebSocket-Key:" ) )
        {
            std::vector<std::string> tokens = Tokenize( str, " " );
            m_WebSocketKey = tokens[1];
            ReplaceStringInPlace( m_WebSocketKey, "\r\n", "" );
            process_handshake_key( m_WebSocketKey );
        }
        
        m_StreamBuf.consume( bytes_transferred );
        
        if( bytes_transferred == 2 )
        {
            SendWebsocketResponse();
            ReadWebsocketMessage();
        }
        else
        {
            asio::async_read_until( m_Socket, m_StreamBuf, "\r\n", std::bind( &TcpConnection::handle_request_line, this, std::placeholders::_1, std::placeholders::_2));
        }
    }
    else
    {
        PRINT_VAR( ec.message() );
    }
}

//-----------------------------------------------------------------------------
void handle_writes(const asio::error_code& error, size_t bytes_transferred)
{
    std::string errorStr = error.message();
    PRINT_VAR( error.message().c_str() );
    PRINT_VAR( bytes_transferred );
}

//-----------------------------------------------------------------------------
void TcpConnection::SendWebsocketResponse()
{
    const char* p = "HTTP/1.1 101 Switching Protocols\r\nConnection: upgrade\r\nSec-WebSocket-Accept: ";
    const char* s = "\r\nUpgrade: websocket\r\n\r\n";
    
    std::string response = p + m_WebSocketKey + s;

    
    asio::async_write( m_Socket, asio::buffer( response.data(), response.size() ), handle_writes );
}

//-----------------------------------------------------------------------------
void TcpConnection::ReadWebsocketHandshake()
{
    asio::async_read_until( m_Socket, m_StreamBuf, "\r\n", std::bind( &TcpConnection::handle_request_line, this, std::placeholders::_1, std::placeholders::_2));
}

//-----------------------------------------------------------------------------
void TcpConnection::ReadPayload()
{
    if( IsWebSocketHandshakeMessage( m_Message ) )
    {
        ReadWebsocketHandshake();
        DecodeMessage(Message(Msg_WebSocketHandshake));
        return;
    }

    if( m_Message.m_Size == 0 )
    {
        m_Message.m_Data = nullptr;
        ReadFooter();
    }
    else
    {
        m_Payload.resize(m_Message.m_Size);
        asio::async_read( m_Socket, asio::buffer( m_Payload.data(), m_Message.m_Size ),

        [this](asio::error_code ec, std::size_t bytes_transferred)
        {
            if (!ec)
            {
                m_NumBytesReceived += bytes_transferred;
                m_Message.m_Data = m_Payload.data();
				ReadFooter();
            }
            else
            {
                PRINT_VAR( ec.message() );
                m_Socket.close();
            }
        }
        );
    }
}

//-----------------------------------------------------------------------------
void TcpConnection::ReadFooter()
{
	unsigned int footer = 0;
	asio::read(m_Socket, asio::buffer(&footer, 4));
	assert(footer == MAGIC_FOOT_MSG);
    m_NumBytesReceived += 4;
	DecodeMessage(m_Message);
	ReadMessage();
}

//-----------------------------------------------------------------------------
void TcpConnection::DecodeMessage( Message & a_Message )
{
    GTcpServer->GetServer()->RegisterConnection( this->shared_from_this() );
    GTcpServer->Receive( a_Message );
}
