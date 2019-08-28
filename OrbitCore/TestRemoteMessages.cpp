#include "TestRemoteMessages.h"
#include "OrbitProcess.h"
#include "OrbitModule.h"
#include "OrbitFunction.h"
#include "TcpServer.h"
#include "TcpClient.h"
#include "Serialization.h"

#include <fstream>
#include <streambuf>
#include <sstream>
#include <iostream>

//-----------------------------------------------------------------------------
TestRemoteMessages& TestRemoteMessages::Get()
{
    static TestRemoteMessages instance;
    return instance;
}

//-----------------------------------------------------------------------------
TestRemoteMessages::TestRemoteMessages()
{

}

//-----------------------------------------------------------------------------
TestRemoteMessages::~TestRemoteMessages()
{
}

void TestRemoteMessages::Init()
{

}

//-----------------------------------------------------------------------------
template <class T> std::string SerializeObject(T& a_Object)
{
    std::stringstream buffer;
    cereal::BinaryOutputArchive archive( buffer );
    archive(a_Object);
    PRINT_VAR(buffer.str().size());
    return buffer.str();
}

//-----------------------------------------------------------------------------
void TestRemoteMessages::Run()
{
    Process process;
    process.m_Name = L"process.m_Name";
    process.m_FullName = L"process.m_FullName";
    process.SetID( 22 );
    process.m_Is64Bit = true;
    process.m_DebugInfoLoaded = true;
    process.m_IsRemote = true;
    process.m_ThreadIds.insert(0);
    process.m_ThreadIds.insert(1);
    process.m_ThreadIds.insert(2);

    std::string processData = SerializeObject(process);
    GTcpClient->Send(Msg_RemoteModules, (void*)processData.data(), processData.size());

    Module module;
    module.m_Name = L"module.m_Name";
    module.m_FullName = L"module.m_FullName";
    module.m_PdbName = L"module.m_PdbName";
    module.m_Directory = L"module.m_Directory";
    module.m_PrettyName = L"module.m_PrettyName";
    module.m_AddressRange = L"module.m_AddressRange";
    module.m_DebugSignature = L"module.m_DebugSignature";
    module.m_ModuleHandle = (HMODULE)1;
    module.m_AddressStart = 2;
    module.m_AddressEnd = 3;
    module.m_EntryPoint = 4;
    module.m_FoundPdb = true;
    
    module.m_Selected = true;
    module.m_Loaded = true;
    module.m_PdbSize = 110;

    std::string moduleData = SerializeObject(module);
    GTcpClient->Send(Msg_RemoteModules, (void*)moduleData.data(), moduleData.size());
}

//-----------------------------------------------------------------------------
void TestRemoteMessages::SetupMessageHandlers()
{
    GTcpServer->SetCallback( Msg_RemoteProcesses, [=]( const Message & a_Msg )
    {
        PRINT_VAR(a_Msg.m_Size);
        std::istringstream buffer(std::string(a_Msg.m_Data, a_Msg.m_Size));
        cereal::BinaryInputArchive inputAr( buffer );
        Process process;
        inputAr(process);
        PRINT_VAR(process.GetName());
    } );

    GTcpServer->SetCallback( Msg_RemoteModules, [=]( const Message & a_Msg )
    {
        PRINT_VAR(a_Msg.m_Size);
        std::istringstream buffer(std::string(a_Msg.m_Data, a_Msg.m_Size));
        cereal::BinaryInputArchive inputAr( buffer );
        Module module;
        inputAr(module);
        PRINT_VAR(module.m_Name);
    } );

    GTcpServer->SetCallback( Msg_RemoteFunctions, [=]( const Message & a_Msg )
    {
        PRINT_VAR(a_Msg.m_Size);
        std::istringstream buffer(std::string(a_Msg.m_Data, a_Msg.m_Size));
        cereal::BinaryInputArchive inputAr( buffer );
        Function function;
        inputAr(function);
        PRINT_VAR(function.m_Name);
    } );
}