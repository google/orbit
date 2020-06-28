// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PluginManager.h"

#include "../OrbitPlugin/OrbitSDK.h"
#include "Core.h"
#include "TcpServer.h"

PluginManager GPluginManager;

//-----------------------------------------------------------------------------
void PluginManager::Initialize() {
#ifdef _WIN32
  std::string dir = Path::GetPluginPath();
  std::vector<std::string> plugins = Path::ListFiles(dir, ".dll");

  for (std::string& file : plugins) {
    HMODULE module = LoadLibraryA(file.c_str());
    using function = Orbit::Plugin*();
    function* func = reinterpret_cast<function*>(
        GetProcAddress(module, "CreateOrbitPlugin"));
    if (func) {
      Orbit::Plugin* newPlugin = func();
      static int count = 0;
      newPlugin->SetPluginID(count++);
      m_Plugins.push_back(newPlugin);
    }
  }

  GTcpServer->AddCallback(Msg_UserData, [=](const Message& a_Msg) {
    this->OnReceiveUserData(a_Msg);
  });
  GTcpServer->AddCallback(Msg_OrbitData, [=](const Message& a_Msg) {
    this->OnReceiveOrbitData(a_Msg);
  });
#endif
}

//-----------------------------------------------------------------------------
void PluginManager::OnReceiveUserData(const Message& a_Msg) {
  if (a_Msg.GetType() == Msg_UserData) {
    const Orbit::UserData* userData =
        static_cast<const Orbit::UserData*>(a_Msg.m_Data);
    // TODO: remove const_cast
    const_cast<Orbit::UserData*>(userData)->m_Data = const_cast<uint8_t*>(
        static_cast<const uint8_t*>(a_Msg.m_Data) + sizeof(Orbit::UserData));

    for (Orbit::Plugin* plugin : m_Plugins) {
      plugin->ReceiveUserData(userData);
    }
  }
}

//-----------------------------------------------------------------------------
void PluginManager::OnReceiveOrbitData(const Message& a_Msg) {
  if (a_Msg.GetType() == Msg_OrbitData) {
    const Orbit::Data* orbitData =
        static_cast<const Orbit::Data*>(a_Msg.GetData());
    // TODO: Remove const_cast
    const_cast<Orbit::Data*>(orbitData)->m_Data = const_cast<uint8_t*>(
        static_cast<const uint8_t*>(a_Msg.GetData()) + sizeof(Orbit::Data));

    for (Orbit::Plugin* plugin : m_Plugins) {
      plugin->ReceiveOrbitData(orbitData);
    }
  }
}
