// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.



#include "OrbitAsio.h"

#include "Tcp.h"
#include "VariableTracing.h"

//-----------------------------------------------------------------------------
TcpService::TcpService() { m_IoService = std::make_unique<asio::io_service>(); }

//-----------------------------------------------------------------------------
TcpService::~TcpService() { PRINT_FUNC; }

//-----------------------------------------------------------------------------
TcpSocket::~TcpSocket() { PRINT_FUNC; }
