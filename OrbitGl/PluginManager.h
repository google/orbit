// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <vector>

namespace Orbit {
class Plugin;
}
class Message;

class PluginManager {
 public:
  void Initialize();

  void OnReceiveUserData(const Message& a_Msg);
  void OnReceiveOrbitData(const Message& a_Msg);

  std::vector<Orbit::Plugin*> m_Plugins;
};

extern PluginManager GPluginManager;