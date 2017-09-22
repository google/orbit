//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Core.h"
#include "ProcessUtils.h"
#include <string>

using namespace std;

class Injection
{
public:
    Injection();

    bool Inject( const wstring & a_Dll, const Process & a_Process, const std::string & ProcName );
    DWORD GetProcessID() const { return m_InjectedProcessID; }
    HANDLE GetProcessHandle() const { return m_InjectedProcessHandle; }

    static HANDLE GetTargetProcessHandle( const string & a_Target, DWORD & o_ProcessID );
    static HMODULE WINAPI GetRemoteModuleHandle( HANDLE hProcess, LPCSTR lpModuleName );
    static FARPROC WINAPI GetRemoteProcAddress( HANDLE hProcess, HMODULE hModule, LPCSTR lpProcName, UINT Ordinal = 0, BOOL UseOrdinal = FALSE );

protected:
    void* RemoteWrite( const char* a_Data, int a_NumBytes );
    void* RemoteWrite( const std::string & a_String );
    void* RemoteWrite( const std::wstring & a_String );

private:
    DWORD  m_InjectedProcessID;
    HANDLE m_InjectedProcessHandle;
};

