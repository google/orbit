// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Path.h"

#include <filesystem>
#include <fstream>

#include "PrintVar.h"
#include "Utils.h"

#ifdef _WIN32
#include <direct.h>

#include "Shlobj.h"
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

std::string Path::base_path_;
bool Path::is_packaged_;

void Path::Init() { GetBasePath(); }

std::string Path::GetExecutableName() {
#ifdef _WIN32
  WCHAR cwBuffer[2048] = {0};
  LPWSTR pszBuffer = cwBuffer;
  DWORD dwMaxChars = _countof(cwBuffer);
  DWORD dwLength = 0;

  dwLength = ::GetModuleFileNameW(NULL, pszBuffer, dwMaxChars);
  std::wstring exeFullName = std::wstring(pszBuffer);

  // Clean up "../" inside full path
  wchar_t buffer[MAX_PATH];
  GetFullPathName(exeFullName.c_str(), MAX_PATH, buffer, nullptr);
  exeFullName = buffer;

  std::replace(exeFullName.begin(), exeFullName.end(), '\\', '/');
  return ws2s(exeFullName);
#else
  char result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  std::string fullPath = std::string(result, (count > 0) ? count : 0);
  return fullPath;
#endif
}

std::string Path::GetExecutablePath() {
  std::string fullPath = GetExecutableName();
  std::string path = fullPath.substr(0, fullPath.find_last_of("/")) + "/";
  return path;
}

bool Path::FileExists(const std::string& file) {
  struct stat statbuf;
  int ret = stat(file.c_str(), &statbuf);
  return ret == 0;
}

uint64_t Path::FileSize(const std::string& file) {
  struct stat stat_buf;
  int ret = stat(file.c_str(), &stat_buf);
  return ret == 0 ? stat_buf.st_size : 0;
}

bool Path::DirExists(const std::string& dir) {
#if _WIN32
  DWORD ftyp = GetFileAttributesA(dir.c_str());
  if (ftyp == INVALID_FILE_ATTRIBUTES) return false;

  if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;

  return false;
#else
  // TODO: also use stat here?
  DIR* pDir;
  bool exists = false;
  pDir = opendir(dir.c_str());
  if (pDir != nullptr) {
    exists = true;
    (void)closedir(pDir);
  }

  return exists;
#endif
}

std::string Path::GetBasePath() {
  if (!base_path_.empty()) {
    return base_path_;
  }

  std::string exePath = GetExecutablePath();
  base_path_ = exePath.substr(0, exePath.find("bin/"));
  is_packaged_ = DirExists(GetBasePath() + "text");

  return base_path_;
}

std::string Path::GetDllPath(bool a_Is64Bit) {
  std::string basePath = GetBasePath();

  return basePath + GetDllName(a_Is64Bit);
}

std::string Path::GetDllName(bool a_Is64Bit) {
  return a_Is64Bit ? "Orbit64.dll" : "Orbit32.dll";
}

static std::string CreateAndGetConfigPath() {
  std::string configDir = Path::JoinPath({Path::GetAppDataPath(), "config"});
  std::filesystem::create_directory(configDir);
  return configDir;
}

std::string Path::GetFileMappingFileName() {
  return Path::JoinPath({CreateAndGetConfigPath(), "FileMapping.txt"});
}

std::string Path::GetSymbolsFileName() {
  return Path::JoinPath({CreateAndGetConfigPath(), "SymbolPaths.txt"});
}

std::string Path::GetCachePath() {
  std::string cacheDir = Path::JoinPath({Path::GetAppDataPath(), "cache"});
  std::filesystem::create_directory(cacheDir);
  return cacheDir;
}

std::string Path::GetPresetPath() {
  std::string presetDir = Path::JoinPath({Path::GetAppDataPath(), "presets"});
  std::filesystem::create_directory(presetDir);
  return presetDir;
}

std::string Path::GetPluginPath() {
  std::string presetDir = Path::JoinPath({Path::GetAppDataPath(), "plugins"});
  std::filesystem::create_directory(presetDir);
  return presetDir;
}

std::string Path::GetCapturePath() {
  std::string captureDir = Path::JoinPath({Path::GetAppDataPath(), "output"});
  std::filesystem::create_directory(captureDir);
  return captureDir;
}

std::string Path::GetDumpPath() {
  std::string captureDir = Path::JoinPath({Path::GetAppDataPath(), "dumps"});
  std::filesystem::create_directory(captureDir);
  return captureDir;
}

std::string Path::GetFileName(const std::string& a_FullName) {
  std::string FullName = a_FullName;
  std::replace(FullName.begin(), FullName.end(), '\\', '/');
  auto index = FullName.find_last_of("/");
  if (index != std::string::npos) {
    std::string FileName = FullName.substr(FullName.find_last_of("/") + 1);
    return FileName;
  }

  return a_FullName;
}

std::string Path::GetFileNameNoExt(const std::string& a_FullName) {
  return StripExtension(GetFileName(a_FullName));
}

std::string Path::StripExtension(const std::string& a_FullName) {
  size_t index = a_FullName.find_last_of(".");
  if (index != std::string::npos) return a_FullName.substr(0, index);
  return a_FullName;
}

std::string Path::GetExtension(const std::string& a_FullName) {
  // returns ".ext" (includes point)
  size_t index = a_FullName.find_last_of(".");
  if (index != std::string::npos)
    return a_FullName.substr(index, a_FullName.length());
  return "";
}

std::string Path::GetDirectory(const std::string& a_FullName) {
  std::string FullName = a_FullName;
  std::replace(FullName.begin(), FullName.end(), '\\', '/');
  auto index = FullName.find_last_of("/");
  if (index != std::string::npos) {
    std::string FileName = FullName.substr(0, FullName.find_last_of("/") + 1);
    return FileName;
  }

  return "";
}

std::string Path::GetParentDirectory(std::string a_Directory) {
  if (a_Directory.empty()) return "";
  std::replace(a_Directory.begin(), a_Directory.end(), '\\', '/');
  wchar_t lastChar = a_Directory.c_str()[a_Directory.size() - 1];
  if (lastChar == '/') a_Directory.erase(a_Directory.size() - 1);

  return GetDirectory(a_Directory);
}

std::string Path::JoinPath(const std::vector<std::string>& parts) {
  if (parts.empty()) {
    return "";
  }
  std::filesystem::path joined{parts[0]};
  for (size_t i = 1; i < parts.size(); ++i) {
    joined.append(parts[i]);
  }
  return joined.string();
}

std::string Path::GetAppDataPath() {
#ifdef WIN32
  std::string appData = GetEnvVar("APPDATA");
  std::string path = Path::JoinPath({appData, "OrbitProfiler"});
#else
  std::string path = Path::JoinPath({Path::GetHome(), ".orbitprofiler"});
#endif
  std::filesystem::create_directory(path);
  return path;
}

std::string Path::GetLogFilePath() {
  std::string logsDir = Path::JoinPath({Path::GetAppDataPath(), "logs"});
  std::filesystem::create_directory(logsDir);
  return Path::JoinPath({logsDir, "Orbit.log"});
}

std::string Path::GetIconsPath() {
  static std::string icons_path = JoinPath({GetExecutablePath(), "icons"});
  return icons_path;
}

#ifdef __linux__
std::string Path::GetHome() {
  std::string home = GetEnvVar("HOME") + "/";
  return home;
}

std::string Path::GetServiceLogFilePath() {
  std::string logsDir = Path::JoinPath({"/", "var", "log"});
  std::filesystem::create_directory(logsDir);
  return Path::JoinPath({logsDir, "OrbitService.log"});
}
#endif

void Path::Dump() {
  PRINT_VAR(GetExecutableName());
  PRINT_VAR(GetExecutablePath());
  PRINT_VAR(GetBasePath());
  PRINT_VAR(GetDllPath(true));
  PRINT_VAR(GetDllName(true));
  PRINT_VAR(GetDllPath(false));
  PRINT_VAR(GetDllName(false));
  PRINT_VAR(GetFileMappingFileName());
  PRINT_VAR(GetSymbolsFileName());
  PRINT_VAR(GetCachePath());
  PRINT_VAR(GetPresetPath());
  PRINT_VAR(GetPluginPath());
  PRINT_VAR(GetCapturePath());
  PRINT_VAR(GetDumpPath());
  PRINT_VAR(GetAppDataPath());
}

std::vector<std::string> Path::ListFiles(
    const std::string& directory,
    std::function<bool(const std::string&)> filter) {
  std::vector<std::string> files;

  for (const auto& file : std::filesystem::directory_iterator(directory)) {
    if (std::filesystem::is_regular_file(file)) {
      std::string path(file.path().string());
      if (filter(path)) files.push_back(path);
    }
  }

  return files;
}

std::vector<std::string> Path::ListFiles(const std::string& directory,
                                         const std::string& filter) {
  return ListFiles(directory, [&](const std::string& file_name) {
    return absl::StrContains(file_name, filter);
  });
}
