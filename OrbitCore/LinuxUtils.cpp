//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include <iostream>
#include <fstream>

#include "LinuxUtils.h"
#include "Utils.h"
#include "PrintVar.h"
#include "OrbitModule.h"
#include "Path.h"
#include "ScopeTimer.h"
#include "Capture.h"
#include "Callstack.h"
#include "SamplingProfiler.h"
#include "EventBuffer.h"
#include "Capture.h"
#include "ScopeTimer.h"
#include "OrbitProcess.h"
#include "ConnectionManager.h"
#include "TcpClient.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <linux/types.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <string.h>
#include <iomanip>
#include <signal.h>
#include <sys/wait.h>
#include <sstream>
#include <string>
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#include <regex>

namespace LinuxUtils {

//-----------------------------------------------------------------------------
std::vector<std::string> ListModules( uint32_t a_PID )
{
    std::vector<std::string> modules;
    std::string result = ExecuteCommand( Format("cat /proc/%u/maps", a_PID).c_str() );

    std::stringstream ss(result);
    std::string line;
    while(std::getline(ss,line,'\n'))
    {
        modules.push_back(line);
    }

    return modules;
}

//-----------------------------------------------------------------------------
std::string ExecuteCommand( const char* a_Cmd ) 
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(a_Cmd, "r"), pclose);
    if (!pipe) {
        std::cout << "Could not open pipe" << std::endl;
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

//-----------------------------------------------------------------------------
void ListModules( uint32_t a_PID, std::map< uint64_t, std::shared_ptr<Module> > & o_ModuleMap )
{
    std::map<std::string, std::shared_ptr<Module>> modules;
    std::vector<std::string> result = ListModules( a_PID );
    for( const std::string& line : result )
    {
        std::vector<std::string> tokens = Tokenize( line ); 
        if( tokens.size() == 6 )
        {
            std::string& moduleName = tokens[5];
            
            auto addresses = Tokenize(tokens[0], "-");
            if( addresses.size() == 2 )
            {
                auto iter = modules.find(moduleName);
                std::shared_ptr<Module> module = 
                    (iter == modules.end()) ? std::make_shared<Module>() : iter->second;

                uint64_t start = std::stoull(addresses[0], nullptr, 16);
                uint64_t end = std::stoull(addresses[1], nullptr, 16);
                
                if( module->m_AddressStart == 0 || start < module->m_AddressStart )
                    module->m_AddressStart = start;
                if( end > module->m_AddressEnd )
                    module->m_AddressEnd = end;
                
                module->m_FullName = moduleName;
                module->m_Name = ws2s(Path::GetFileName( s2ws(module->m_FullName) ));
                module->m_Directory = ws2s(Path::GetDirectory( s2ws(module->m_FullName) ));
                auto prettyName = module->GetPrettyName();
                modules[moduleName] = module;
            }
        }
    }

    for( auto& iter : modules )
    {
        auto module = iter.second;
        o_ModuleMap[module->m_AddressStart] = module;
    }
}

//-----------------------------------------------------------------------------
uint32_t GetPID(const char* a_Name)
{
    std::string command = Format("ps -A | grep %s", a_Name);
    std::string result = ExecuteCommand( command.c_str() );
    auto tokens = Tokenize(result);
    if( !tokens.empty() )
        return (uint32_t)atoi(tokens[0].c_str());
    std::cout << "Could not find process " << a_Name;
    return 0;
}

//-----------------------------------------------------------------------------
std::unordered_map<uint32_t, float> GetCpuUtilization()
{
    std::unordered_map<uint32_t, float> processMap;
    std::string result = ExecuteCommand("top -b -n 1 | sed -n '8, 1000{s/^ *//;s/ *$//;s/  */,/gp;};1000q'");
    std::stringstream ss(result);
    std::string line;

    while (std::getline(ss, line, '\n'))
    {
        auto tokens = Tokenize(line, ",");
        if( tokens.size() > 8)
        {
            uint32_t pid = atoi(tokens[0].c_str());
            float cpu = atof(tokens[8].c_str());
            processMap[pid] = cpu;
        }
    }

    return processMap;
}

//-----------------------------------------------------------------------------
bool Is64Bit( uint32_t a_PID )
{
    std::string result = ExecuteCommand(Format("file -L /proc/%u/exe", a_PID).c_str());
    return Contains(result, "64-bit");
}

//-----------------------------------------------------------------------------
std::string Demangle(const char* name)
{
    int status = 0;
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };

    return (status==0) ? res.get() : name;
}

//-----------------------------------------------------------------------------
double GetSecondsFromNanos( uint64_t a_Nanos )
{
    return 0.000000001*(double)a_Nanos;
}

//-----------------------------------------------------------------------------
void DumpClocks()
{
    uint64_t realTime     = OrbitTicks(CLOCK_REALTIME);
    uint64_t monotonic    = OrbitTicks(CLOCK_MONOTONIC);
    uint64_t monotonicRaw = OrbitTicks(CLOCK_MONOTONIC_RAW);
    uint64_t bootTime     = OrbitTicks(CLOCK_BOOTTIME);
    uint64_t tai          = OrbitTicks(CLOCK_TAI);
    
    printf("    realTime: %lu \n", realTime);
    printf("   monotonic: %lu \n", monotonic);
    printf("monotonicRaw: %lu \n", monotonicRaw);
    printf("    bootTime: %lu \n", bootTime);
    printf("         tai: %lu \n\n", tai);

    printf("    realTime: %f \n",   GetSecondsFromNanos(realTime));
    printf("   monotonic: %f \n",   GetSecondsFromNanos(monotonic));
    printf("monotonicRaw: %f \n",   GetSecondsFromNanos(monotonicRaw));
    printf("    bootTime: %f \n",   GetSecondsFromNanos(bootTime));
    printf("         tai: %f \n\n", GetSecondsFromNanos(tai));
}

//-----------------------------------------------------------------------------
void StreamCommandOutput(const char* a_Cmd, std::function<void(const std::string&)> a_Callback, bool* a_ExitRequested)
{
    std::cout << "Starting output stream for command" << a_Cmd << std::endl;

    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(a_Cmd, "r"), pclose);
    
    if (!pipe) 
    {
        std::cout << "Could not open pipe" << std::endl;
    }
    
    while ( fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr && 
            !(*a_ExitRequested) )
    {
        a_Callback(buffer.data());
    }
    
    std::cout << "end stream" << std::endl;
}

//-----------------------------------------------------------------------------
std::string GetModuleBaseName( const std::string& a_Module )
{
    const std::regex re("([a-z]*).*");
    std::smatch match;
    assert(std::regex_match(a_Module, match, re));
    return match.str(1);
}

}

