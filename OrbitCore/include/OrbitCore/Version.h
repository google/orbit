//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>

//-----------------------------------------------------------------------------
class OrbitVersion {
 public:
  static std::string GetVersion();
  static void CheckForUpdate();
  static void CheckForUpdateThread();
  static bool IsDev();

  static bool s_NeedsUpdate;
  static std::string s_LatestVersion;
};