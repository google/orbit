// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CrashService/CrashServiceImpl.h"

#include "GrpcProtos/services.pb.h"
#include "OrbitBase/Logging.h"

namespace orbit_crash_service {

using orbit_grpc_protos::CrashOrbitServiceRequest;
using orbit_grpc_protos::CrashOrbitServiceResponse;

static void InfiniteRecursion(int num) {
  if (num != 1) {
    InfiniteRecursion(num);
  }
  ORBIT_LOG("%i", num);
}

grpc::Status CrashServiceImpl::CrashOrbitService(grpc::ServerContext* /*context*/,
                                                 const CrashOrbitServiceRequest* request,
                                                 CrashOrbitServiceResponse* /*response*/) {
  switch (request->crash_type()) {
    case CrashOrbitServiceRequest::CHECK_FALSE: {
      ORBIT_CHECK(false);
      break;
    }
    case CrashOrbitServiceRequest::STACK_OVERFLOW: {
      InfiniteRecursion(0);
      break;
    }
    default:
      break;
  }

  return grpc::Status::OK;
}

}  // namespace orbit_crash_service
