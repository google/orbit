// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TestRemoteMessages.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <streambuf>

#include "OrbitFunction.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "Serialization.h"
#include "TcpClient.h"
#include "TcpServer.h"

//-----------------------------------------------------------------------------
TestRemoteMessages& TestRemoteMessages::Get() {
  static TestRemoteMessages instance;
  return instance;
}

//-----------------------------------------------------------------------------
TestRemoteMessages::TestRemoteMessages() {}

//-----------------------------------------------------------------------------
TestRemoteMessages::~TestRemoteMessages() {}

void TestRemoteMessages::Init() { SetupMessageHandlers(); }

//-----------------------------------------------------------------------------
template <class T>
std::string SerializeObject(T& a_Object) {
  std::stringstream buffer;
  cereal::BinaryOutputArchive archive(buffer);
  archive(a_Object);
  PRINT_VAR(buffer.str().size());
  return buffer.str();
}

//-----------------------------------------------------------------------------
void TestRemoteMessages::Run() {
  Process process;
  process.m_Name = "process.m_Name";
  process.m_FullPath = "process.m_FullPath";
  process.m_CmdLine = "process.m_CmdLine";
  process.SetID(22);
  process.m_Is64Bit = true;
  process.m_DebugInfoLoaded = true;
  process.m_IsRemote = true;
  process.m_ThreadIds.insert(0);
  process.m_ThreadIds.insert(1);
  process.m_ThreadIds.insert(2);

  std::string processData = SerializeObjectHumanReadable(process);
  PRINT_VAR(processData);
  GTcpClient->Send(Msg_RemoteProcess, processData.data(), processData.size());

  Module module;
  module.m_Name = "module.m_Name";
  module.m_FullName = "module.m_FullName";
  module.m_PdbName = "module.m_PdbName";
  module.m_Directory = "module.m_Directory";
  module.m_PrettyName = "module.m_PrettyName";
  module.m_AddressRange = "module.m_AddressRange";
  module.m_DebugSignature = "module.m_DebugSignature";
  module.m_ModuleHandle = reinterpret_cast<HMODULE>(static_cast<uintptr_t>(1));
  module.m_AddressStart = 2;
  module.m_AddressEnd = 3;
  module.m_EntryPoint = 4;
  module.loadable_ = true;

  module.m_Selected = true;
  module.loaded_ = true;
  module.m_PdbSize = 110;

  std::string moduleData = SerializeObjectHumanReadable(module);
  GTcpClient->Send(Msg_RemoteModule, moduleData.data(), moduleData.size());

  Function function{"m_Name", "m_PrettyName", 1, 2, 3, "m_File", 4};
  function.SetId(5);
  function.SetParentId(6);
  function.SetCallingConvention(7);

  std::string functionData = SerializeObjectHumanReadable(function);
  GTcpClient->Send(Msg_RemoteFunctions, functionData.data(),
                   functionData.size());
}

//-----------------------------------------------------------------------------
void TestRemoteMessages::SetupMessageHandlers() {
  GTcpServer->AddCallback(Msg_RemoteProcess, [=](const Message& a_Msg) {
    PRINT_VAR(a_Msg.m_Size);
    std::istringstream buffer(a_Msg.GetDataAsString());
    cereal::JSONInputArchive inputAr(buffer);
    Process process;
    inputAr(process);
    PRINT_VAR(process.GetName());
  });

  GTcpServer->AddCallback(Msg_RemoteModule, [=](const Message& a_Msg) {
    PRINT_VAR(a_Msg.m_Size);
    std::istringstream buffer(a_Msg.GetDataAsString());
    cereal::JSONInputArchive inputAr(buffer);
    Module module;
    inputAr(module);
    PRINT_VAR(module.m_Name);
  });

  GTcpServer->AddCallback(Msg_RemoteFunctions, [=](const Message& a_Msg) {
    PRINT_VAR(a_Msg.m_Size);
    std::istringstream buffer(a_Msg.GetDataAsString());
    cereal::JSONInputArchive inputAr(buffer);
    Function function;
    inputAr(function);
    PRINT_VAR(function.Name());
  });
}
