// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#pragma once

#include <string>

class tcp_server;
class TcpSocket;
class TcpService;

namespace asio {
class io_context;
}

#define MAGIC_FOOT_MSG 0xf007ba11
#define ORBIT_HOST "localhost"
#define ORBIT_EXE_NAME "Orbit.exe"
