//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>

#include <client/crashpad_client.h>

class CrashHandler {
 public:
  explicit CrashHandler(const std::string& dump_path,
                        const std::string& handler_path);
  void DumpWithoutCrash() const;

 private:
  inline static bool is_init_{false};
  crashpad::CrashpadClient crashpad_client_;
};