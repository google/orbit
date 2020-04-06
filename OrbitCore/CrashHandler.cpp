//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "CrashHandler.h"

#include "Core.h"
#include "OrbitDbgHelp.h"
#include "ScopeTimer.h"
#include "TcpClient.h"

#include "OrbitBase/Logging.h"

#ifdef _WIN32

//-----------------------------------------------------------------------------
CrashHandler::CrashHandler(const std::string& dump_path,
                           const std::string& handler_path) {
  CHECK(!is_init_);
  is_init_ = true;

  // Creates a new CrashpadClient instance that directs crashes to crashpad
  // handler. Minidump files will be written to dump_path.

  base::FilePath dump_file_path(s2ws(dump_path));
  base::FilePath handler_file_path(s2ws(handler_path));

  crashpad_client_.StartHandler(handler_file_path,
                                /*database=*/dump_file_path,
                                /*metrics_dir=*/dump_file_path, /*url=*/{},
                                /*annotations=*/{}, /*arguments=*/{},
                                /*restartable=*/true,
                                /*asynchronous_start=*/true);
  crashpad_client_.WaitForHandlerStart(INFINITE);
}

//-----------------------------------------------------------------------------
void CrashHandler::DumpWithoutCrash() {
  crashpad::NativeCPUContext cpu_context;
  crashpad::CaptureContext(&cpu_context);
  crashpad_client_.DumpWithoutCrash(cpu_context);
}

#endif
