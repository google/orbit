//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <atomic>
#include <string>

#ifdef _WIN32
#include <client/crashpad_client.h>
#endif

class CrashHandler {
 public:
  explicit CrashHandler(const std::string& dump_path,
                        const std::string& handler_path);
  void DumpWithoutCrash();

 private:
  inline static bool is_init_{false};
#ifdef _WIN32
  crashpad::CrashpadClient crashpad_client_;
#endif
};