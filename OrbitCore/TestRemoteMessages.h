// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#pragma once

#include <memory.h>

#include <string>

#include "BaseTypes.h"
#include "SerializationMacros.h"

class TestRemoteMessages {
 public:
  static TestRemoteMessages& Get();

  TestRemoteMessages();
  ~TestRemoteMessages();

  void Init();
  void Run();

 protected:
  void SetupMessageHandlers();
};
