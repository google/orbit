// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <libfuzzer/libfuzzer_macro.h>

#include "ClientData/ModuleData.h"
#include "FuzzingUtils/ProtoFuzzer.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"

using orbit_client_data::ModuleData;
using orbit_grpc_protos::ModuleSymbols;

ORBIT_DEFINE_PROTO_FUZZER(const ModuleSymbols& symbols) {
  ModuleData module{orbit_grpc_protos::ModuleInfo{}};
  module.AddSymbols(symbols);
}