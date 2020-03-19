//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "BaseTypes.h"

struct Module;

//-----------------------------------------------------------------------------
namespace LinuxUtils {
std::string ExecuteCommand(const char* a_Cmd);
void StreamCommandOutput(const char* a_Cmd,
                         std::function<void(const std::string&)> a_Callback,
                         bool* a_ExitRequested);
std::vector<std::string> ListModules(pid_t a_PID);
uint64_t GetTracePointID(const char* a_Group, const char* a_Event);
void ListModules(pid_t a_PID,
                 std::map<uint64_t, std::shared_ptr<Module> >& o_ModuleMap);
std::unordered_map<uint32_t, float> GetCpuUtilization();
bool Is64Bit(pid_t a_PID);
void DumpClocks();
std::string GetKernelVersionStr();
uint32_t GetKernelVersion();
bool IsKernelOlderThan(const char* a_Version);
}  // namespace LinuxUtils
