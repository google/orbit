//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "asio.hpp"

using namespace asio::ip;

//-----------------------------------------------------------------------------
class TcpService
{
public:
    TcpService();
    ~TcpService();
    asio::io_context* m_IoService;
};

//-----------------------------------------------------------------------------
class TcpSocket
{
public:
    TcpSocket() : m_Socket( nullptr ){}
    TcpSocket( tcp::socket* a_Socket ) : m_Socket( a_Socket ){};
    ~TcpSocket();
    tcp::socket* m_Socket;
};