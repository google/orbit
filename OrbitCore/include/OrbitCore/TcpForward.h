//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>

class tcp_server;
class TcpSocket;
class TcpService;

namespace asio {
class io_context;
}

#define MAX_WS_HEADER_LENGTH 14
#define MAGIC_FOOT_MSG 0xf007ba11
#define ORBIT_HOST "localhost"
#define ORBIT_EXE_NAME "Orbit.exe"
