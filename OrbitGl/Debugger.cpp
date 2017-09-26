//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Core.h"
#include "Debugger.h"
#include "OrbitDbgHelp.h"
#include "TcpServer.h"
#include "App.h"
#include <psapi.h>
#include <thread>
#include <string>
#include "Params.h"

//-----------------------------------------------------------------------------
Debugger::Debugger() : m_LoopReady(false)
{
}

//-----------------------------------------------------------------------------
Debugger::~Debugger()
{
}

//-----------------------------------------------------------------------------
void Debugger::LaunchProcess( const std::wstring & a_ProcessName, const std::wstring & a_WorkingDir, const std::wstring & a_Args )
{
    std::thread t( &Debugger::DebuggerThread, this, a_ProcessName, a_WorkingDir, a_Args );
    t.detach();
}

//-----------------------------------------------------------------------------
void Debugger::MainTick()
{
    if( m_LoopReady )
    {
        if( GOrbitApp->Inject( m_ProcessID ) )
        {
            GTcpServer->Send( Msg_WaitLoop, m_WaitLoop );
            GOrbitApp->RequestThaw();
        }
        m_LoopReady = false;
    }
}

//-----------------------------------------------------------------------------
void Debugger::SendThawMessage()
{
    GTcpServer->Send( Msg_ThawMainThread, m_WaitLoop );
}

#define BUFSIZE 512
//-----------------------------------------------------------------------------
std::string GetFileNameFromHandle( HANDLE hFile )
{
    BOOL bSuccess = FALSE;
    TCHAR pszFilename[MAX_PATH + 1];
    HANDLE hFileMap;

    std::wstring strFilename;

    // Get the file size.
    DWORD dwFileSizeHi = 0;
    DWORD dwFileSizeLo = GetFileSize( hFile, &dwFileSizeHi );

    if( dwFileSizeLo == 0 && dwFileSizeHi == 0 )
    {
        return FALSE;
    }

    // Create a file mapping object.
    hFileMap = CreateFileMapping( hFile,
        NULL,
        PAGE_READONLY,
        0,
        1,
        NULL );

    if( hFileMap )
    {
        // Create a file mapping to get the file name.
        void* pMem = MapViewOfFile( hFileMap, FILE_MAP_READ, 0, 0, 1 );

        if( pMem )
        {
            if( GetMappedFileName( GetCurrentProcess(),
                pMem,
                pszFilename,
                MAX_PATH ) )
            {

                // Translate path with device name to drive letters.
                TCHAR szTemp[BUFSIZE];
                szTemp[0] = '\0';

                if( GetLogicalDriveStrings( BUFSIZE - 1, szTemp ) )
                {
                    TCHAR szName[MAX_PATH];
                    TCHAR szDrive[3] = TEXT( " :" );
                    BOOL bFound = FALSE;
                    TCHAR* p = szTemp;

                    do
                    {
                        // Copy the drive letter to the template string
                        *szDrive = *p;

                        // Look up each device name
                        if( QueryDosDevice( szDrive, szName, MAX_PATH ) )
                        {
                            size_t uNameLen = _tcslen( szName );

                            if( uNameLen < MAX_PATH )
                            {
                                bFound = _tcsnicmp( pszFilename, szName,
                                    uNameLen ) == 0;

                                if( bFound )
                                {
                                    strFilename = Format( L"%s%s", szDrive, pszFilename + uNameLen );
                                }
                            }
                        }

                        // Go to the next NULL character.
                        while( *p++ );
                    } while( !bFound && *p ); // end of string
                }
            }
            bSuccess = TRUE;
            UnmapViewOfFile( pMem );
        }

        CloseHandle( hFileMap );
    }

    return ws2s(strFilename);
}

HANDLE GMainThreadHandle = 0;
HANDLE hProcess = 0;
void* startAddress = 0;

//-----------------------------------------------------------------------------
void Debugger::DebuggerThread( const std::wstring & a_ProcessName, const std::wstring & a_WorkingDir, const std::wstring & a_Args )
{
    SetThreadName( GetCurrentThreadId(), "Debugger" );

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &si, sizeof( si ) );
    si.cb = sizeof( si );
    ZeroMemory( &pi, sizeof( pi ) );

    std::wstring dir = a_WorkingDir.size() ? a_WorkingDir : Path::GetDirectory( a_ProcessName );
    std::wstring args = a_ProcessName + L" " + a_Args;
    TCHAR commandline[MAX_PATH + 1];
    int numChars = (int)min( (size_t)MAX_PATH, args.size() );
    memcpy( commandline, args.c_str(), numChars*sizeof(TCHAR) );
    commandline[numChars] = 0;

    bool success = CreateProcess( a_ProcessName.c_str()
                                , commandline
                                , NULL
                                , NULL
                                , FALSE
                                , DEBUG_ONLY_THIS_PROCESS
                                , NULL
                                , dir.c_str()
                                , &si
                                , &pi ) != 0;

    std::string strEventMessage;
    std::map<LPVOID, std::string> DllNameMap;
    DEBUG_EVENT debug_event = { 0 };
    bool bContinueDebugging = true;
    DWORD dwContinueStatus = DBG_CONTINUE;
    bool detach = false;

    while( bContinueDebugging )
    {
        if( !WaitForDebugEvent( &debug_event, INFINITE ) )
            return;

        switch( debug_event.dwDebugEventCode )
        {
        case CREATE_PROCESS_DEBUG_EVENT:
        {
            strEventMessage = GetFileNameFromHandle( debug_event.u.CreateProcessInfo.hFile );
            GMainThreadHandle = debug_event.u.CreateProcessInfo.hThread;

            hProcess = debug_event.u.CreateProcessInfo.hProcess;
            startAddress = debug_event.u.CreateProcessInfo.lpStartAddress;
            m_ProcessID = GetProcessId(hProcess);

            if( GParams.m_StartPaused )
            {
                // Copy original 2 bytes before installing busy loop
                m_WaitLoop.m_Address = (DWORD64)startAddress;
                m_WaitLoop.m_ThreadId = GetThreadId( debug_event.u.CreateProcessInfo.hThread );
                ReadProcessMemory( hProcess, startAddress, &m_WaitLoop.m_OriginalBytes, 2, NULL );

                // Install busy loop 
                unsigned char loop[] = { 0xEB, 0xFE };
                SIZE_T numWritten = 0;
                WriteProcessMemory( hProcess, startAddress, &loop, sizeof( loop ), &numWritten );
                FlushInstructionCache( hProcess, startAddress, sizeof( loop ) );

                m_LoopReady = true;
            }
            detach = true;
        }
        break;

        case CREATE_THREAD_DEBUG_EVENT:
        {
            strEventMessage = Format( "Thread 0x%x (Id: %d) created at: 0x%x",
                debug_event.u.CreateThread.hThread,
                debug_event.dwThreadId,
                debug_event.u.CreateThread.lpStartAddress ); // Thread 0xc (Id: 7920) created at: 0x77b15e58

            break;
        }
        case EXIT_THREAD_DEBUG_EVENT:

            strEventMessage = Format( "The thread %d exited with code: %d",
                debug_event.dwThreadId,
                debug_event.u.ExitThread.dwExitCode );	// The thread 2760 exited with code: 0

            break;

        case EXIT_PROCESS_DEBUG_EVENT:
            strEventMessage = Format( "0x%x", debug_event.u.ExitProcess.dwExitCode );
            bContinueDebugging = false;
            break;

        case LOAD_DLL_DEBUG_EVENT:
        {
            strEventMessage = GetFileNameFromHandle( debug_event.u.LoadDll.hFile );

            DllNameMap.insert( std::make_pair( debug_event.u.LoadDll.lpBaseOfDll, strEventMessage ) );

            strEventMessage += Format( "%x", debug_event.u.LoadDll.lpBaseOfDll );
        }
        break;

        case UNLOAD_DLL_DEBUG_EVENT:
            strEventMessage = Format( "%s", DllNameMap[debug_event.u.UnloadDll.lpBaseOfDll] );
            break;

        case OUTPUT_DEBUG_STRING_EVENT:
        {
            OUTPUT_DEBUG_STRING_INFO & DebugString = debug_event.u.DebugString;
            // LPSTR p = ;

            WCHAR *msg = new WCHAR[DebugString.nDebugStringLength];
            ZeroMemory( msg, DebugString.nDebugStringLength );

            ReadProcessMemory( pi.hProcess, DebugString.lpDebugStringData, msg, DebugString.nDebugStringLength, NULL );

            if( DebugString.fUnicode )
                strEventMessage = ws2s(msg);
            else
                strEventMessage = (LPSTR)msg;

            delete[]msg;
        }

        break;

        case EXCEPTION_DEBUG_EVENT:
        {
            EXCEPTION_DEBUG_INFO& exception = debug_event.u.Exception;
            switch( exception.ExceptionRecord.ExceptionCode )
            {
            case STATUS_BREAKPOINT:
            {
                /*bool suspendThreadresult = SuspendThread( GMainThreadHandle ) >= 0;
                PRINT_VAR(suspendThreadresult);*/

                strEventMessage = "Break point";
                break;
            }

            default:
                if( exception.dwFirstChance == 1 )
                {
                    strEventMessage = Format( "First chance exception at %x, exception-code: 0x%08x",
                        exception.ExceptionRecord.ExceptionAddress,
                        exception.ExceptionRecord.ExceptionCode );
                }
                // else
                // { Let the OS handle }


                // There are cases where OS ignores the dwContinueStatus, 
                // and executes the process in its own way.
                // For first chance exceptions, this parameter is not-important
                // but still we are saying that we have NOT handled this event.

                // Changing this to DBG_CONTINUE (for 1st chance exception also), 
                // may cause same debugging event to occur continuously.
                // In short, this debugger does not handle debug exception events
                // efficiently, and let's keep it simple for a while!
                dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
            }

            break;
        }
        }

        PRINT_VAR(strEventMessage);

        ContinueDebugEvent( debug_event.dwProcessId,
            debug_event.dwThreadId,
            dwContinueStatus );

        // Reset
        dwContinueStatus = DBG_CONTINUE;

        if( detach )
        {
            DebugActiveProcessStop( GetProcessId(hProcess) );
            bContinueDebugging = false;
        }
    }
}
