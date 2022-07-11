// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_LIVE_FUNCTIONS_INTERFACE_H_
#define DATA_VIEWS_LIVE_FUNCTIONS_INTERFACE_H_

#include <stdint.h>

#include "ClientData/FunctionInfo.h"
#include "ClientData/ScopeId.h"

namespace orbit_data_views {

class LiveFunctionsInterface {
 public:
  virtual ~LiveFunctionsInterface() = default;

  virtual void AddIterator(orbit_client_data::ScopeId instrumented_function_scope_id,
                           const orbit_client_data::FunctionInfo* function) = 0;
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_LIVE_FUNCTIONS_INTERFACE_H_
