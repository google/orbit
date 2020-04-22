//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "CrashHandler.h"

#include "Core.h"
#include "OrbitBase/Logging.h"
#include "OrbitDbgHelp.h"
#include "ScopeTimer.h"
#include "TcpClient.h"
#include "Version.h"
#include "client/crash_report_database.h"
#include "client/settings.h"

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
                           const std::string& handler_path,
                           const std::string& crash_server_url,
                           bool is_upload_enabled) {
  CHECK(!is_init_);
  is_init_ = true;

  // Creates a new CrashpadClient instance that directs crashes to crashpad
  // handler. Minidump files will be written to dump_path and sent to
  // crash_server.

  const base::FilePath dump_file_path(StringTypeConverter<>()(dump_path));
  const base::FilePath handler_file_path(StringTypeConverter<>()(handler_path));

  const std::map<std::string, std::string> annotations = {
      {"product", "OrbitProfiler"}, {"version", OrbitVersion::GetVersion()}};

  const std::vector<std::string> arguments = {"--no-rate-limit"};

  // set user preferences for dumps submission to collection server
  std::unique_ptr<crashpad::CrashReportDatabase> crash_report_db =
      crashpad::CrashReportDatabase::Initialize(dump_file_path);
  if (crash_report_db != nullptr && crash_report_db->GetSettings() != nullptr) {
    crash_report_db->GetSettings()->SetUploadsEnabled(is_upload_enabled);
  }

  crashpad_client_.StartHandler(handler_file_path,
                                /*database=*/dump_file_path,
                                /*metrics_dir=*/dump_file_path,
                                crash_server_url, annotations, arguments,
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
