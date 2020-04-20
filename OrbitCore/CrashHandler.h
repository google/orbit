//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#ifndef ORBIT_CORE_CRASH_HANDLER_H_
#define ORBIT_CORE_CRASH_HANDLER_H_

#include <client/crashpad_client.h>

#include <string>

class CrashHandler {
 public:
  explicit CrashHandler(const std::string& dump_path,
                        const std::string& handler_path,
                        const std::string& crash_server_url,
                        bool is_upload_enabled);
  void DumpWithoutCrash() const;

 private:
  inline static bool is_init_{false};
  crashpad::CrashpadClient crashpad_client_;
};

#endif  // ORBIT_CORE_CRASH_HANDLER_H_