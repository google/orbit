// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_GGP_CLIENT_GGP_H_
#define ORBIT_CLIENT_GGP_CLIENT_GGP_H_

#include "grpcpp/grpcpp.h"

#include "ClientGgpOptions.h"
#include "OrbitCaptureClient/CaptureClient.h"
#include "OrbitCaptureClient/CaptureListener.h"

class ClientGgp {
  public:
    ClientGgp(ClientGgpOptions&& options);
    bool InitClient();
    void StartCapture();
    void EndCapture();
    void SaveCapture();

  private:
    ClientGgpOptions options_;

    std::shared_ptr<grpc::Channel> grpc_channel_;

    //std::unique_ptr<CaptureClient> capture_client_;
};

#endif // ORBIT_CLIENT_GGP_CLIENT_GGP_H_