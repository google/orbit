// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_GGP_CLIENT_GGP_H_
#define ORBIT_CLIENT_GGP_CLIENT_GGP_H_

#include <string>

#include "capture_data.pb.h"
#include "grpcpp/grpcpp.h"

#include "Callstack.h"
#include "EventBuffer.h"
#include "KeyAndString.h"
#include "ScopeTimer.h"

#include "ClientGgpOptions.h"
#include "OrbitCaptureClient/CaptureClient.h"
#include "OrbitCaptureClient/CaptureListener.h"

class ClientGgp : public CaptureListener {
  public:
    ClientGgp(ClientGgpOptions&& options);
    bool InitClient();
    void StartCapture();
    void EndCapture();
    void SaveCapture();

    // CaptureListener implementation
    void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;
    void OnKeyAndString(uint64_t key, std::string str) override;
    void OnCallstack(CallStack callstack) override;
    void OnCallstackEvent(
      orbit_client_protos::CallstackEvent callstack_event) override;
    void OnThreadName(int32_t thread_id, std::string thread_name) override;
    void OnAddressInfo(
      orbit_client_protos::LinuxAddressInfo address_info) override;

  private:
    ClientGgpOptions options_;

    std::shared_ptr<grpc::Channel> grpc_channel_;
    
    std::unique_ptr<CaptureClient> capture_client_;
};

#endif // ORBIT_CLIENT_GGP_CLIENT_GGP_H_