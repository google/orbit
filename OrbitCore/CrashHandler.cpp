//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "CrashHandler.h"

#include "Core.h"
#include "OrbitDbgHelp.h"
#include "ScopeTimer.h"
#include "TcpClient.h"

#ifdef _WIN32
#include <Shlwapi.h>

//-----------------------------------------------------------------------------
CrashHandler::CrashHandler(const std::wstring& dump_path) {
  bool not_init = false;
  if (is_init_.compare_exchange_strong(not_init, true)) {
    Init(dump_path);
  }
}

void CrashHandler::Init(const std::wstring& dump_path) {
  // Creates a new CrashpadClient instance that directs crashes to crashpad
  // handler. Minidump files will be written to dump_path.

  TCHAR exe_path[MAX_PATH];
  GetModuleFileName(NULL, exe_path, MAX_PATH);
  PathRemoveFileSpec(exe_path);
  PathAppend(exe_path, L"crashpad_handler.exe");

  base::FilePath dump_file_path(dump_path);
  base::FilePath handler_file_path(exe_path);

  std::map<std::string, std::string> annotations;
  std::vector<std::string> arguments;

  crashpad_client_.StartHandler(
      handler_file_path, /*database=*/dump_file_path,
      /*metrics_dir=*/dump_file_path, /*url=*/std::string{},
      /*annotations=*/std::map<std::string, std::string>{},
      /*arguments=*/std::vector<std::string>{}, /*restartable=*/true,
      /*asynchronous_start=*/true);
  crashpad_client_.WaitForHandlerStart(INFINITE);
}

//-----------------------------------------------------------------------------
void CrashHandler::SendMiniDump() {
  crashpad::NativeCPUContext cpu_context;
  crashpad::CaptureContext(&cpu_context);
  crashpad_client_.DumpWithoutCrash(cpu_context);
}

#endif
