//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ProcessUtils.h"
#include <tlhelp32.h>
#include "Log.h"

// Is64BitProcess function taken from Very Sleepy
#ifdef _WIN64
typedef BOOL WINAPI Wow64GetThreadContext_t(__in     HANDLE hThread, __inout  PWOW64_CONTEXT lpContext);
typedef DWORD WINAPI Wow64SuspendThread_t(__in  HANDLE hThread);
Wow64GetThreadContext_t *fn_Wow64GetThreadContext = (Wow64GetThreadContext_t *)GetProcAddress(GetModuleHandle(L"kernel32"), "Wow64GetThreadContext");
Wow64SuspendThread_t *fn_Wow64SuspendThread = (Wow64SuspendThread_t *)GetProcAddress(GetModuleHandle(L"kernel32"), "Wow64SuspendThread");
#endif

//-----------------------------------------------------------------------------
bool ProcessUtils::Is64Bit(HANDLE hProcess)
{
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
    return false;
}

//-----------------------------------------------------------------------------
ProcessList::ProcessList()
{
    Refresh();
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
    m_Processes.clear();
    std::unordered_map< DWORD, std::shared_ptr< Process > > previousProcessesMap = m_ProcessesMap;
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
                process->m_Name = processinfo.szExeFile;
                process->SetID(processinfo.th32ProcessID);
                
                /*TCHAR fullPath[1024];
                DWORD pathSize = 1024;
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
                    process->m_FullName = moduleEntry.szExePath;

                    CloseHandle(moduleSnapshot);
                }

                m_Processes.push_back( process );
            }

            m_ProcessesMap[processinfo.th32ProcessID] = m_Processes.back();

        } while (Process32Next(snapshot, &processinfo));
    }

    SortByCPU();
    CloseHandle(snapshot);
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
    for( std::shared_ptr< Process > & process : m_Processes )
    {
        process->UpdateCpuTime();
    }
}

//-----------------------------------------------------------------------------
bool ProcessList::Contains( DWORD a_PID ) const
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
