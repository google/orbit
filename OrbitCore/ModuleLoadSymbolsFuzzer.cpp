// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <libfuzzer/libfuzzer_macro.h>

#include "OrbitModule.h"
#include "symbol.pb.h"

DEFINE_PROTO_FUZZER(const ModuleSymbols& symbols) {
  Module module{};
  module.LoadSymbols(symbols);
}