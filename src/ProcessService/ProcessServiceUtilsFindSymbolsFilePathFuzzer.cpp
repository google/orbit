// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <libfuzzer/libfuzzer_macro.h>

#include "FuzzingUtils/ProtoFuzzer.h"
#include "GrpcProtos/services.pb.h"
#include "ProcessServiceUtils.h"

ORBIT_DEFINE_PROTO_FUZZER(const orbit_grpc_protos::GetDebugInfoFileRequest& request) {
  (void)orbit_process_service::FindSymbolsFilePath(request);
}