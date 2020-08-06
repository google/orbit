// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_GGP_CLIENT_GGP_OPTIONS_H_
#define ORBIT_CLIENT_GGP_CLIENT_GGP_OPTIONS_H_

#include <string>

// The struct used to store Orbit Ggp Client options
// The default values are set by main()
struct ClientGgpOptions {
  // gRPC connection string
  std::string grpc_server_address;
  int32_t capture_pid;
};

#endif // ORBIT_CLIENT_GGP_CLIENT_GGP_OPTIONS_H_