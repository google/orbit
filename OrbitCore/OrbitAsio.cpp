//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "OrbitAsio.h"
#include "Tcp.h"
#include "VariableTracing.h"

//-----------------------------------------------------------------------------
TcpService::TcpService() { m_IoService = new asio::io_service(); }

//-----------------------------------------------------------------------------
TcpService::~TcpService() { PRINT_FUNC; }

//-----------------------------------------------------------------------------
TcpSocket::~TcpSocket() { PRINT_FUNC; }
