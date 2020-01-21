//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ProcessUtils.h"
#include "Log.h"
#include <memory>
#include <string>
#include <algorithm>
#include <locale>
#include <iterator>

#ifdef _WIN32
#include <tlhelp32.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h> // for opendir(), readdir(), closedir()
#include <sys/stat.h> // for stat()

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <fstream>
#include <streambuf>

#define PROC_DIRECTORY "/proc/"
#define CASE_SENSITIVE    1
#define CASE_INSENSITIVE  0
#define EXACT_MATCH       1
#define INEXACT_MATCH     0
#endif

// Is64BitProcess function taken from Very Sleepy
#ifdef _WIN64
typedef BOOL WINAPI Wow64GetThreadContext_t(__in     HANDLE hThread, __inout  PWOW64_CONTEXT lpContext);
typedef DWORD WINAPI Wow64SuspendThread_t(__in  HANDLE hThread);
Wow64GetThreadContext_t *fn_Wow64GetThreadContext = (Wow64GetThreadContext_t *)GetProcAddress(GetModuleHandle(L"kernel32"), "Wow64GetThreadContext");
Wow64SuspendThread_t *fn_Wow64SuspendThread = (Wow64SuspendThread_t *)GetProcAddress(GetModuleHandle(L"kernel32"), "Wow64SuspendThread");
#endif

int IsNumeric(const char* ccharptr_CharacterList)
{
    for ( ; *ccharptr_CharacterList; ccharptr_CharacterList++)
        if (*ccharptr_CharacterList < '0' || *ccharptr_CharacterList > '9')
            return 0; // false
    return 1; // true
}

#ifndef _WIN32
namespace  {
std::string PrettifyProcessName(const std::string &processName)
{
    // Idea: processName contains (if not changed by the process) a
    // null-separated list of arguments, while the first is the path to the
    // executable as called by the user. We check wether that first string is a
    // file path. If so, we search for the last forward slash in this string and
    // copy from there up to the end of the whole string. So we get the
    // executable name and the arguments.

    auto resultingName = [&]() -> std::string {
        const auto end = std::find(processName.begin(), processName.end(), '\0');

        const std::string filepath{processName.begin(), end};

        struct stat st{};
        const auto result = stat(filepath.c_str(), &st);

        if (result == 0) {
            // We found a file
            const auto begin = std::find(std::make_reverse_iterator(end),
                                         processName.rend(), '/');
            return std::string{begin.base(), processName.end()};
        }
        return processName;
    }();

    std::replace(resultingName.begin(), resultingName.end(), '\0', ' ');
    return resultingName;
}
} // namespace
#endif

//-----------------------------------------------------------------------------
bool ProcessUtils::Is64Bit(HANDLE hProcess)
{
#ifdef _WIN32
    // https://github.com/VerySleepy/verysleepy/blob/master/src/utils/osutils.cpp

    typedef BOOL WINAPI IsWow64Process_t(__in   HANDLE hProcess, __out  PBOOL Wow64Process);
    static bool first = true;
    static IsWow64Process_t *IsWow64ProcessPtr = NULL;

#ifndef _WIN64
    static BOOL isOn64BitOs = 0;
#endif

    if (first) {
        first = false;
        IsWow64ProcessPtr = (IsWow64Process_t *)GetProcAddress(GetModuleHandle(L"kernel32"), "IsWow64Process");
#ifndef _WIN64
        if (!IsWow64ProcessPtr)
            return false;
        IsWow64ProcessPtr(GetCurrentProcess(), &isOn64BitOs);
#endif
    }

#ifndef _WIN64
    if (!isOn64BitOs) {
        return false;
    }
#endif

    if (IsWow64ProcessPtr) {
        BOOL isWow64Process;
        if (IsWow64ProcessPtr(hProcess, &isWow64Process) && !isWow64Process) {
            return true;
        }
    }

#endif
    return false;
}

//-----------------------------------------------------------------------------
ProcessList::ProcessList()
{
}

//-----------------------------------------------------------------------------
void ProcessList::Clear()
{
    m_Processes.clear();
    m_ProcessesMap.clear();
}

//-----------------------------------------------------------------------------
void ProcessList::Refresh()
{
#ifdef _WIN32
    m_Processes.clear();
    std::unordered_map< uint32_t, std::shared_ptr< Process > > previousProcessesMap = m_ProcessesMap;
    m_ProcessesMap.clear();

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPMODULE/*| TH32CS_SNAPTHREAD*/, 0);
    PROCESSENTRY32 processinfo;
    processinfo.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &processinfo))
    {
        do
        {
            if (processinfo.th32ProcessID == GetCurrentProcessId())
                continue;

//#ifndef _WIN64
//            // If the process is 64 bit, skip it.
//            if (Is64BitProcess(process_handle)) {
//                CloseHandle(process_handle);
//                continue;
//            }
//#else
//            // Skip 32 bit processes on system that does not have the needed functions (Windows XP 64).
//            if (/*TODO:*/ (fn_Wow64SuspendThread == NULL || fn_Wow64GetThreadContext == NULL) && !ProcessUtils::Is64Bit(process_handle)) {
//                CloseHandle(process_handle);
//                continue;
//            }
//#endif

            auto it = previousProcessesMap.find( processinfo.th32ProcessID );
            if( it != previousProcessesMap.end() )
            {
                // Add existing process
                m_Processes.push_back( it->second );
            }
            else
            {
                // Process was not there previously
                auto process = std::make_shared<Process>();
                process->m_Name = ws2s(processinfo.szExeFile);
                process->SetID(processinfo.th32ProcessID);
                
                /*TCHAR fullPath[1024];
                uint32_t pathSize = 1024;
                QueryFullProcessImageName( process->GetHandle(), 0, fullPath, &pathSize );*/

                // Full path
                HANDLE moduleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processinfo.th32ProcessID);
                if( moduleSnapshot != INVALID_HANDLE_VALUE)
                {
                    MODULEENTRY32 moduleEntry;
                    moduleEntry.dwSize = sizeof(MODULEENTRY32);
                    BOOL res = Module32First( moduleSnapshot, &moduleEntry );
                    if( !res )
                    {
                        ORBIT_ERROR;
                    }
                    process->m_FullName = ws2s(moduleEntry.szExePath);

                    CloseHandle(moduleSnapshot);
                }

                m_Processes.push_back( process );
            }

            m_ProcessesMap[processinfo.th32ProcessID] = m_Processes.back();

        } while (Process32Next(snapshot, &processinfo));
    }
    CloseHandle(snapshot);

#else
    m_Processes.clear();
    struct dirent* de_DirEntity = NULL;
    DIR* dir_proc = NULL;

    dir_proc = opendir(PROC_DIRECTORY);
    if (dir_proc == NULL)
    {
        perror("Couldn't open the " PROC_DIRECTORY " directory") ;
        return;
    }

    while ( (de_DirEntity = readdir(dir_proc)) )
    {
        if (de_DirEntity->d_type == DT_DIR)
        {
            if (IsNumeric(de_DirEntity->d_name))
            {
                int pid = atoi(de_DirEntity->d_name);
                std::string commandLinePath = Format("%s%s/cmdline",PROC_DIRECTORY, de_DirEntity->d_name);

                std::ifstream inFile(commandLinePath);
                if(!inFile.fail())
                {
                    std::stringstream buffer;
                    buffer << inFile.rdbuf();
                    inFile.close();
                    const std::string& processName = buffer.str();

                    if( processName.empty() )
                        continue;
                        
                    auto iter = m_ProcessesMap.find(pid);
                    std::shared_ptr<Process> process = nullptr;
                    if( iter == m_ProcessesMap.end() )
                    {
                        process = std::make_shared<Process>();
                        process->m_FullName = processName;
                        process->m_Name = PrettifyProcessName(processName);
                        process->SetID(pid);
                        m_ProcessesMap[pid] = process;
                    }
                    else
                    {
                        process = iter->second;
                    }

                    m_Processes.push_back(process);
                }
            }
        }
    }
    closedir(dir_proc) ;
#endif
}

//-----------------------------------------------------------------------------
void ProcessList::SortByID()
{
    std::sort( m_Processes.begin(), m_Processes.end(), []( std::shared_ptr<Process> & a_P1, std::shared_ptr<Process> & a_P2 ){ return a_P1->GetID() < a_P2->GetID(); } );
}

//-----------------------------------------------------------------------------
void ProcessList::SortByName()
{
    std::sort( m_Processes.begin(), m_Processes.end(), []( std::shared_ptr<Process> & a_P1, std::shared_ptr<Process> & a_P2 ){ return a_P1->m_Name < a_P2->m_Name; } );
}

//-----------------------------------------------------------------------------
void ProcessList::SortByCPU()
{
    std::sort(m_Processes.begin(), m_Processes.end(), []( std::shared_ptr<Process> & a_P1, std::shared_ptr<Process> & a_P2 ){ return a_P1->GetCpuUsage() < a_P2->GetCpuUsage(); });
}

//-----------------------------------------------------------------------------
void ProcessList::UpdateCpuTimes()
{
#ifdef WIN32
    for( std::shared_ptr< Process > & process : m_Processes )
    {
        process->UpdateCpuTime();
    }
#else
    std::unordered_map<uint32_t, float> processMap = LinuxUtils::GetCpuUtilization();
    for( std::shared_ptr< Process > & process : m_Processes )
    {
        uint32_t pid = process->GetID();
        process->SetCpuUsage( processMap[pid] );
    }
#endif
}

//-----------------------------------------------------------------------------
bool ProcessList::Contains(uint32_t a_PID ) const
{
	for( const std::shared_ptr< Process > & process : m_Processes )
	{
		if( process->GetID() == a_PID )
		{
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
void ProcessList::SetRemote( bool a_Value )
{
    for (std::shared_ptr< Process > & process : m_Processes)
    {
        process->SetIsRemote(true);
    }
}

//-----------------------------------------------------------------------------
std::shared_ptr<Process> ProcessList::GetProcess(uint32_t a_PID)
{
    std::shared_ptr<Process> result = nullptr;
    auto iter = m_ProcessesMap.find(a_PID);
    if (iter != m_ProcessesMap.end())
        result = iter->second;
    return result;
}


//-----------------------------------------------------------------------------
ORBIT_SERIALIZE( ProcessList, 0 )
{
    ORBIT_NVP_VAL( 0, m_Processes );
    ORBIT_NVP_VAL( 0, m_ProcessesMap );
}