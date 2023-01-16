// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <libfuzzer/libfuzzer_macro.h>

#include "ClientData/ModuleData.h"
#include "ClientData/ModuleIdentifierProvider.h"
#include "FuzzingUtils/ProtoFuzzer.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"

using orbit_client_data::ModuleData;
using orbit_grpc_protos::ModuleSymbols;

ORBIT_DEFINE_PROTO_FUZZER(const ModuleSymbols& symbols) {
  orbit_client_data::ModuleIdentifierProvider module_identifier_provider;
  static const orbit_client_data::ModuleIdentifier kModuleIdentifier =
      module_identifier_provider.CreateModuleIdentifier("/a/path/", "build_id");
  ModuleData module{orbit_grpc_protos::ModuleInfo{}, kModuleIdentifier};
  module.AddSymbols(symbols);
}