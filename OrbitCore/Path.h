//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <functional>
#include <string>
#include <vector>

class Path {
 public:
  static void Init();

  static std::string GetExecutableName();
  static std::string GetExecutablePath();
  static std::string GetBasePath();
  static std::string GetOrbitAppPdb();
  static std::string GetDllPath(bool a_Is64Bit);
  static std::string GetDllName(bool a_Is64Bit);
  static std::string GetParamsFileName();
  static std::string GetFileMappingFileName();
  static std::string GetSymbolsFileName();
  static std::string GetLicenseName();
  static std::string GetCachePath();
  static std::string GetPresetPath();
  static std::string GetPluginPath();
  static std::string GetCapturePath();
  static std::string GetDumpPath();
  static std::string GetTmpPath();
  static std::string GetProgramFilesPath();
  static std::string GetAppDataPath();
  static std::string GetMainDrive();
  static std::string GetSourceRoot();
  static std::string GetHome();
  static void Dump();

  static std::string GetFileName(const std::string& a_FullName);
  static std::string GetFileNameNoExt(const std::string& a_FullName);
  static std::string StripExtension(const std::string& a_FullName);
  static std::string GetExtension(const std::string& a_FullName);
  static std::string GetDirectory(const std::string& a_FullName);
  static std::string GetParentDirectory(std::string a_FullName);

  static bool FileExists(const std::string& a_File);
  static uint64_t FileSize(const std::string& a_File);
  static bool DirExists(const std::string& a_Dir);
  static void MakeDir(const std::string& a_Directory);
  static bool IsSourceFile(const std::string& a_File);
  static bool IsPackaged() { return is_packaged_; }

  static std::vector<std::string> ListFiles(
      const std::string& directory,
      std::function<bool(const std::string&)> filter = [](const std::string&) {
        return true;
      });
  static std::vector<std::string> ListFiles(const std::string& directory,
                                            const std::string& filter);

 private:
  static std::string base_path_;
  static bool is_packaged_;
};
