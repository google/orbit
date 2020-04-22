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
  GTcpClient->Send(Msg_RemoteProcess, (void*)processData.data(),
                   processData.size());

  Module module;
  module.m_Name = "module.m_Name";
  module.m_FullName = "module.m_FullName";
  module.m_PdbName = "module.m_PdbName";
  module.m_Directory = "module.m_Directory";
  module.m_PrettyName = "module.m_PrettyName";
  module.m_AddressRange = "module.m_AddressRange";
  module.m_DebugSignature = "module.m_DebugSignature";
  module.m_ModuleHandle = (HMODULE)1;
  module.m_AddressStart = 2;
  module.m_AddressEnd = 3;
  module.m_EntryPoint = 4;
  module.m_FoundPdb = true;

  module.m_Selected = true;
  module.m_Loaded = true;
  module.m_PdbSize = 110;

  std::string moduleData = SerializeObjectHumanReadable(module);
  GTcpClient->Send(Msg_RemoteModule, (void*)moduleData.data(),
                   moduleData.size());

  Function function;
  function.SetName("m_Name");
  function.SetPrettyName("m_PrettyName");
  // This will initialize m_PrettyLowerName to Lower(pretty_name_)
  function.Lower();
  function.SetModule("m_Module");
  function.SetFile("m_File");
  function.SetProbe("m_Probe");
  function.SetAddress(1);
  function.SetSize(3);
  function.SetId(4);
  function.SetParentId(5);
  function.SetLine(6);
  function.SetCallingConvention(7);

  std::string functionData = SerializeObjectHumanReadable(function);
  GTcpClient->Send(Msg_RemoteFunctions, (void*)functionData.data(),
                   functionData.size());
}

//-----------------------------------------------------------------------------
void TestRemoteMessages::SetupMessageHandlers() {
  GTcpServer->AddCallback(Msg_RemoteProcess, [=](const Message& a_Msg) {
    PRINT_VAR(a_Msg.m_Size);
    std::istringstream buffer(std::string(a_Msg.m_Data, a_Msg.m_Size));
    cereal::JSONInputArchive inputAr(buffer);
    Process process;
    inputAr(process);
    PRINT_VAR(process.GetName());
  });

  GTcpServer->AddCallback(Msg_RemoteModule, [=](const Message& a_Msg) {
    PRINT_VAR(a_Msg.m_Size);
    std::istringstream buffer(std::string(a_Msg.m_Data, a_Msg.m_Size));
    cereal::JSONInputArchive inputAr(buffer);
    Module module;
    inputAr(module);
    PRINT_VAR(module.m_Name);
  });

  GTcpServer->AddCallback(Msg_RemoteFunctions, [=](const Message& a_Msg) {
    PRINT_VAR(a_Msg.m_Size);
    std::istringstream buffer(std::string(a_Msg.m_Data, a_Msg.m_Size));
    cereal::JSONInputArchive inputAr(buffer);
    Function function;
    inputAr(function);
    PRINT_VAR(function.Name());
  });
}
