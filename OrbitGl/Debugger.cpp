//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#ifdef _WIN32

#include "Debugger.h"

#include <psapi.h>

#include <string>
#include <thread>

#include "App.h"
#include "Core.h"
#include "OrbitDbgHelp.h"
#include "Params.h"
#include "TcpServer.h"

//-----------------------------------------------------------------------------
Debugger::Debugger() : m_LoopReady(false) {}

//-----------------------------------------------------------------------------
Debugger::~Debugger() {}

//-----------------------------------------------------------------------------
void Debugger::LaunchProcess(const std::string& process_name,
                             const std::string& working_dir,
                             const std::string& args) {
  std::thread t(&Debugger::DebuggerThread, this, process_name, working_dir,
                args);
  t.detach();
}

//-----------------------------------------------------------------------------
void Debugger::MainTick() {
  if (m_LoopReady) {
    if (GOrbitApp->Inject(m_ProcessID)) {
      GTcpServer->Send(Msg_WaitLoop, m_WaitLoop);
      GOrbitApp->RequestThaw();
    }
    m_LoopReady = false;
  }
}

//-----------------------------------------------------------------------------
void Debugger::SendThawMessage() {
  GTcpServer->Send(Msg_ThawMainThread, m_WaitLoop);
}

#define BUFSIZE 512
//-----------------------------------------------------------------------------
std::string GetFileNameFromHandle(HANDLE hFile) {
  bool bSuccess = false;
  char pszFilename[MAX_PATH + 1];
  HANDLE hFileMap;

  std::string strFilename;

  // Get the file size.
  DWORD dwFileSizeHi = 0;
  DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi);

  if (dwFileSizeLo == 0 && dwFileSizeHi == 0) {
    return "";
  }

  // Create a file mapping object.
  hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 1, NULL);

  if (hFileMap) {
    // Create a file mapping to get the file name.
    void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

    if (pMem) {
      if (GetMappedFileNameA(GetCurrentProcess(), pMem, pszFilename,
                             MAX_PATH)) {
        // Translate path with device name to drive letters.
        char szTemp[BUFSIZE];
        szTemp[0] = '\0';

        if (GetLogicalDriveStringsA(BUFSIZE - 1, szTemp)) {
          char szName[MAX_PATH];
          char szDrive[3] = " :";
          BOOL bFound = FALSE;
          char* p = szTemp;

          do {
            // Copy the drive letter to the template string
            szDrive[0] = p[0];

            // Look up each device name
            if (QueryDosDeviceA(szDrive, szName, MAX_PATH)) {
              size_t uNameLen = std::strlen(szName);

              if (uNameLen < MAX_PATH) {
                bFound = std::strncmp(pszFilename, szName, uNameLen) == 0;

                if (bFound) {
                  strFilename =
                      absl::StrFormat("%s%s", szDrive, pszFilename + uNameLen);
                }
              }
            }

            // Go to the next NULL character.
            while (*p++)
              ;
          } while (!bFound && *p);  // end of string
        }
      }
      bSuccess = TRUE;
      UnmapViewOfFile(pMem);
    }

    CloseHandle(hFileMap);
  }

  return strFilename;
}

HANDLE GMainThreadHandle = 0;
HANDLE hProcess = 0;
void* startAddress = 0;

//-----------------------------------------------------------------------------
void Debugger::DebuggerThread(const std::string& process_name,
                              const std::string& working_dir,
                              const std::string& args) {
  SetCurrentThreadName(L"Debugger");

  STARTUPINFOA si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  std::string dir =
      !working_dir.empty() ? working_dir : Path::GetDirectory(process_name);
  std::string command_line_str = process_name + " " + args;
  // TODO: commandline is limited to MAX_PATH, should it
  // just report a error if command_line_str length is > MAX_PATH?
  char commandline[MAX_PATH + 1];
  strncpy(commandline, command_line_str.c_str(), MAX_PATH);
  commandline[MAX_PATH] = 0;

  bool success =
      CreateProcessA(process_name.c_str(), commandline, NULL, NULL, FALSE,
                     DEBUG_ONLY_THIS_PROCESS, NULL, dir.c_str(), &si, &pi) != 0;

  UNUSED(success);

  std::string strEventMessage;
  std::map<LPVOID, std::string> DllNameMap;
  DEBUG_EVENT debug_event = {0};
  bool bContinueDebugging = true;
  DWORD dwContinueStatus = DBG_CONTINUE;
  bool detach = false;

  while (bContinueDebugging) {
    if (!WaitForDebugEvent(&debug_event, INFINITE)) return;

    switch (debug_event.dwDebugEventCode) {
      case CREATE_PROCESS_DEBUG_EVENT: {
        strEventMessage =
            GetFileNameFromHandle(debug_event.u.CreateProcessInfo.hFile);
        GMainThreadHandle = debug_event.u.CreateProcessInfo.hThread;

        hProcess = debug_event.u.CreateProcessInfo.hProcess;
        startAddress = debug_event.u.CreateProcessInfo.lpStartAddress;
        m_ProcessID = GetProcessId(hProcess);

        if (GParams.m_StartPaused) {
          // Copy original 2 bytes before installing busy loop
          m_WaitLoop.m_Address = (DWORD64)startAddress;
          m_WaitLoop.m_ThreadId =
              GetThreadId(debug_event.u.CreateProcessInfo.hThread);
          ReadProcessMemory(hProcess, startAddress, &m_WaitLoop.m_OriginalBytes,
                            2, NULL);

          // Install busy loop
          unsigned char loop[] = {0xEB, 0xFE};
          SIZE_T numWritten = 0;
          WriteProcessMemory(hProcess, startAddress, &loop, sizeof(loop),
                             &numWritten);
          FlushInstructionCache(hProcess, startAddress, sizeof(loop));

          m_LoopReady = true;
        }
        detach = true;
      } break;

      case CREATE_THREAD_DEBUG_EVENT:
        // Thread 0xc (Id: 7920) created at: 0x77b15e58
        strEventMessage = absl::StrFormat(
            "Thread 0x%x (Id: %d) created at: 0x%x",
            debug_event.u.CreateThread.hThread, debug_event.dwThreadId,
            debug_event.u.CreateThread.lpStartAddress);

        break;
      case EXIT_THREAD_DEBUG_EVENT:
        // The thread 2760 exited with code: 0
        strEventMessage = absl::StrFormat("The thread %d exited with code: %d",
                                          debug_event.dwThreadId,
                                          debug_event.u.ExitThread.dwExitCode);

        break;

      case EXIT_PROCESS_DEBUG_EVENT:
        strEventMessage =
            absl::StrFormat("0x%x", debug_event.u.ExitProcess.dwExitCode);
        bContinueDebugging = false;
        break;

      case LOAD_DLL_DEBUG_EVENT: {
        strEventMessage = GetFileNameFromHandle(debug_event.u.LoadDll.hFile);

        DllNameMap.insert(
            std::make_pair(debug_event.u.LoadDll.lpBaseOfDll, strEventMessage));

        strEventMessage +=
            absl::StrFormat("%x", debug_event.u.LoadDll.lpBaseOfDll);
        break;
      }

      case UNLOAD_DLL_DEBUG_EVENT:
        strEventMessage = DllNameMap[debug_event.u.UnloadDll.lpBaseOfDll];
        break;

      case OUTPUT_DEBUG_STRING_EVENT: {
        OUTPUT_DEBUG_STRING_INFO& DebugString = debug_event.u.DebugString;
        // LPSTR p = ;

        WCHAR* msg = new WCHAR[DebugString.nDebugStringLength];
        ZeroMemory(msg, DebugString.nDebugStringLength);

        ReadProcessMemory(pi.hProcess, DebugString.lpDebugStringData, msg,
                          DebugString.nDebugStringLength, NULL);

        if (DebugString.fUnicode)
          strEventMessage = ws2s(msg);
        else
          strEventMessage = (LPSTR)msg;

        delete[] msg;
      }

      break;

      case EXCEPTION_DEBUG_EVENT: {
        EXCEPTION_DEBUG_INFO& exception = debug_event.u.Exception;
        switch (exception.ExceptionRecord.ExceptionCode) {
          case STATUS_BREAKPOINT: {
            /*bool suspendThreadresult = SuspendThread( GMainThreadHandle ) >=
            0; PRINT_VAR(suspendThreadresult);*/

            strEventMessage = "Break point";
            break;
          }

          default:
            if (exception.dwFirstChance == 1) {
              strEventMessage = absl::StrFormat(
                  "First chance exception at %x, "
                  "exception-code: 0x%08x",
                  exception.ExceptionRecord.ExceptionAddress,
                  exception.ExceptionRecord.ExceptionCode);
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

    ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId,
                       dwContinueStatus);

    // Reset
    dwContinueStatus = DBG_CONTINUE;

    if (detach) {
      DebugActiveProcessStop(GetProcessId(hProcess));
      bContinueDebugging = false;
    }
  }
}

#endif
