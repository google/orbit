//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "LinuxPerf.h"
#include "BaseTypes.h"
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <thread>
#include <unordered_map>
#include <functional>

struct Module;

//-----------------------------------------------------------------------------
namespace LinuxUtils
{
    std::string ExecuteCommand( const char* a_Cmd );
    void StreamCommandOutput(const char* a_Cmd, std::function<void(const std::string&)> a_Callback, bool* a_ExitRequested);
    std::vector<std::string> ListModules( uint32_t a_PID );
    std::vector<uint64_t> ListThreads( uint32_t a_PID );
    uint64_t GetTracePointID( const char* a_Group, const char* a_Event );
    void ListModules( uint32_t a_PID, std::map< uint64_t, std::shared_ptr<Module> > & o_ModuleMap );
    std::unordered_map<uint32_t, float> GetCpuUtilization();
    bool Is64Bit(uint32_t a_PID);
    std::string Demangle( const char* a_Symbol );
    void DumpClocks();
    std::string GetKernelVersionStr();
    uint32_t GetKernelVersion();
    bool IsKernelOlderThan(const char* a_Version);
}
