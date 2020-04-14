//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "CrashHandler.h"

#include "Core.h"
#include "OrbitBase/Logging.h"
#include "OrbitDbgHelp.h"
#include "ScopeTimer.h"
#include "TcpClient.h"

namespace {
template <typename StringType = base::FilePath::StringType>
struct StringTypeConverter {
  StringType operator()(const std::string& source_string) const {
    return StringType(source_string);
  }
};

template <>
struct StringTypeConverter<std::wstring> {
  std::wstring operator()(const std::string& source_string) const {
    return s2ws(source_string);
  }
};
}  // namespace

//-----------------------------------------------------------------------------
CrashHandler::CrashHandler(const std::string& dump_path,
                           const std::string& handler_path) {
  CHECK(!is_init_);
  is_init_ = true;

  // Creates a new CrashpadClient instance that directs crashes to crashpad
  // handler. Minidump files will be written to dump_path.

  const base::FilePath dump_file_path(StringTypeConverter<>()(dump_path));
  const base::FilePath handler_file_path(StringTypeConverter<>()(handler_path));

  crashpad_client_.StartHandler(handler_file_path,
                                /*database=*/dump_file_path,
                                /*metrics_dir=*/dump_file_path, /*url=*/{},
                                /*annotations=*/{}, /*arguments=*/{},
                                /*restartable=*/true,
                                /*asynchronous_start=*/false);
}

//-----------------------------------------------------------------------------
void CrashHandler::DumpWithoutCrash() const {
  crashpad::NativeCPUContext cpu_context;
  crashpad::CaptureContext(&cpu_context);
#ifdef _WIN32
  crashpad_client_.DumpWithoutCrash(cpu_context);
#else
  crashpad_client_.DumpWithoutCrash(&cpu_context);
#endif
}
