// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Path.h"

#include <filesystem>
#include <string>

#include "OrbitBase/SafeStrerror.h"
#include "Utils.h"
#include "absl/flags/flag.h"

#ifdef _WIN32
#include <direct.h>

#include "Shlobj.h"
#else
#include <dirent.h>
#include <linux/limits.h>
#include <sys/stat.h>

#endif

ABSL_FLAG(std::string, log_dir, "", "Set directory for the log.");

std::string Path::GetExecutablePath() {
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
  char buffer[PATH_MAX];
  ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer));
  if (length == -1) {
    // TODO (161419404): implement error handling
    ERROR("Unable to readlink /proc/self/exe: %s", SafeStrerror(errno));
    return "";
  }

  return std::string(buffer, length);
#endif
}

std::string Path::GetExecutableDir() { return GetDirectory(GetExecutablePath()); }

static std::string CreateAndGetConfigPath() {
  std::string configDir = Path::JoinPath({Path::CreateOrGetOrbitAppDataDir(), "config"});
  std::filesystem::create_directory(configDir);
  return configDir;
}

std::string Path::GetFileMappingFileName() {
  return Path::JoinPath({CreateAndGetConfigPath(), "FileMapping.txt"});
}

std::string Path::GetSymbolsFileName() {
  return Path::JoinPath({CreateAndGetConfigPath(), "SymbolPaths.txt"});
}

std::string Path::CreateOrGetCacheDir() {
  std::string cacheDir = Path::JoinPath({Path::CreateOrGetOrbitAppDataDir(), "cache"});
  std::filesystem::create_directory(cacheDir);
  return cacheDir;
}

std::string Path::CreateOrGetPresetDir() {
  std::string presetDir = Path::JoinPath({Path::CreateOrGetOrbitAppDataDir(), "presets"});
  std::filesystem::create_directory(presetDir);
  return presetDir;
}

std::string Path::CreateOrGetCaptureDir() {
  std::string captureDir = Path::JoinPath({Path::CreateOrGetOrbitAppDataDir(), "output"});
  std::filesystem::create_directory(captureDir);
  return captureDir;
}

std::string Path::CreateOrGetDumpDir() {
  std::string captureDir = Path::JoinPath({Path::CreateOrGetOrbitAppDataDir(), "dumps"});
  std::filesystem::create_directory(captureDir);
  return captureDir;
}

std::string Path::GetFileName(const std::string& file_path) {
  std::string full_name = file_path;
  std::replace(full_name.begin(), full_name.end(), '\\', '/');
  auto index = full_name.find_last_of('/');
  if (index != std::string::npos) {
    std::string file_name = full_name.substr(full_name.find_last_of('/') + 1);
    return file_name;
  }

  return full_name;
}

std::string Path::StripExtension(const std::string& file_path) {
  std::string full_name = file_path;
  std::replace(full_name.begin(), full_name.end(), '\\', '/');
  const std::string extension = GetExtension(full_name);
  return full_name.substr(0, file_path.length() - extension.length());
}

std::string Path::GetExtension(const std::string& file_path) {
  // returns ".ext" (includes point)
  // Perform on file name to make sure we're not detecting "." within the directory path
  const std::string file_name = GetFileName(file_path);
  size_t index = file_name.find_last_of('.');
  if (index != std::string::npos) return file_name.substr(index, file_name.length());
  return "";
}

std::string Path::GetDirectory(const std::string& any_path) {
  std::string FullName = any_path;
  std::replace(FullName.begin(), FullName.end(), '\\', '/');
  auto index = FullName.find_last_of('/');
  if (index != std::string::npos) {
    std::string FileName = FullName.substr(0, FullName.find_last_of('/') + 1);
    return FileName;
  }

  return "";
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

std::string Path::CreateOrGetOrbitAppDataDir() {
#ifdef WIN32
  std::string appData = GetEnvVar("APPDATA");
  std::string path = Path::JoinPath({appData, "OrbitProfiler"});
#else
  std::string path = Path::JoinPath({GetEnvVar("HOME"), ".orbitprofiler"});
#endif
  std::filesystem::create_directory(path);
  return path;
}

std::string Path::GetLogFilePathAndCreateDir() {
  std::string logs_dir;
  if (!absl::GetFlag(FLAGS_log_dir).empty()) {
    logs_dir = absl::GetFlag(FLAGS_log_dir);
  } else {
    logs_dir = Path::JoinPath({Path::CreateOrGetOrbitAppDataDir(), "logs"});
  }
  std::filesystem::create_directory(logs_dir);
  return Path::JoinPath({logs_dir, "Orbit.log"});
}


std::vector<std::string> Path::ListFiles(const std::string& directory,
                                         const std::function<bool(const std::string&)>& filter) {
  std::vector<std::string> files;

  for (const auto& file : std::filesystem::directory_iterator(directory)) {
    if (std::filesystem::is_regular_file(file)) {
      std::string path(file.path().string());
      if (filter(path)) files.push_back(path);
    }
  }

  return files;
}

std::vector<std::string> Path::ListFiles(const std::string& directory, const std::string& filter) {
  return ListFiles(directory, [&](const std::string& file_name) {
    return absl::StrContains(file_name, filter);
  });
}
