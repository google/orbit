//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "asio.hpp"

//-----------------------------------------------------------------------------
class TcpService {
 public:
  TcpService();
  ~TcpService();
  asio::io_context* m_IoService;
};

//-----------------------------------------------------------------------------
class TcpSocket {
 public:
  TcpSocket() : m_Socket(nullptr) {}
  TcpSocket(asio::ip::tcp::socket* a_Socket) : m_Socket(a_Socket){};
  ~TcpSocket();
  asio::ip::tcp::socket* m_Socket;
};