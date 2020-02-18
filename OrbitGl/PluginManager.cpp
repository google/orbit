//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "PluginManager.h"

#include "OrbitCore/OrbitSDK.h"
#include "OrbitCore/Core.h"
#include "OrbitCore/TcpServer.h"

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
    Orbit::UserData* userData = (Orbit::UserData*)a_Msg.m_Data;
    userData->m_Data = (char*)userData + sizeof(Orbit::UserData);

    for (Orbit::Plugin* plugin : m_Plugins) {
      plugin->ReceiveUserData(userData);
    }
  }
}

//-----------------------------------------------------------------------------
void PluginManager::OnReceiveOrbitData(const Message& a_Msg) {
  if (a_Msg.GetType() == Msg_OrbitData) {
    Orbit::Data* orbitData = (Orbit::Data*)a_Msg.m_Data;
    orbitData->m_Data = orbitData + sizeof(Orbit::Data);

    for (Orbit::Plugin* plugin : m_Plugins) {
      plugin->ReceiveOrbitData(orbitData);
    }
  }
}
