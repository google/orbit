//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include <atomic>

#ifdef _WIN32
#include <client/crashpad_client.h>
#endif

class CrashHandler {
 public:
  CrashHandler(const std::wstring& dump_path);
  void SendMiniDump();

 private:
  inline static std::atomic<bool> is_init_{false};
#ifdef _WIN32
  crashpad::CrashpadClient crashpad_client_;
#endif
  void Init(const std::wstring& dump_path);
};