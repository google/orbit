// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Path.h"

#include <absl/flags/flag.h>

#include <filesystem>
#include <string>

#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"

#ifdef WIN32
#include <KnownFolders.h>
#include <shlobj.h>
#endif

ABSL_FLAG(std::string, log_dir, "", "Set directory for the log.");

namespace orbit_core {

static std::string GetEnvVar(const char* variable_name) {
  std::string var;

#ifdef _WIN32
  char* buf = nullptr;
  size_t sz = 0;
  if (_dupenv_s(&buf, &sz, variable_name) == 0 && buf != nullptr) {
    var = buf;
    free(buf);
  }
#else
  char* env = getenv(variable_name);
  if (env != nullptr) var = env;
#endif

  return var;
}

static void CreateDirectoryOrDie(const std::filesystem::path& directory) {
  auto create_directory_result = orbit_base::CreateDirectory(directory);
  if (create_directory_result.has_error()) {
    FATAL("Unable to create directory \"%s\": %s", directory.string(),
          create_directory_result.error().message());
  }
}

static std::filesystem::path CreateAndGetConfigPath() {
  std::filesystem::path config_dir = CreateOrGetOrbitAppDataDir() / "config";
  CreateDirectoryOrDie(config_dir);
  return config_dir;
}

std::filesystem::path GetFileMappingFileName() {
  return CreateAndGetConfigPath() / "FileMapping.txt";
}

std::filesystem::path GetSymbolsFileName() { return CreateAndGetConfigPath() / "SymbolPaths.txt"; }

std::filesystem::path CreateOrGetCacheDir() {
  std::filesystem::path cache_dir = CreateOrGetOrbitAppDataDir() / "cache";
  CreateDirectoryOrDie(cache_dir);
  return cache_dir;
}

std::filesystem::path GetPresetDirPriorTo1_66() { return CreateOrGetOrbitAppDataDir() / "presets"; }

std::filesystem::path GetCaptureDirPriorTo1_66() { return CreateOrGetOrbitAppDataDir() / "output"; }

std::filesystem::path CreateOrGetPresetDir() {
  std::filesystem::path preset_dir = CreateOrGetOrbitUserDataDir() / "presets";
  CreateDirectoryOrDie(preset_dir);
  return preset_dir;
}

std::filesystem::path CreateOrGetCaptureDir() {
  std::filesystem::path capture_dir = CreateOrGetOrbitUserDataDir() / "captures";
  CreateDirectoryOrDie(capture_dir);
  return capture_dir;
}

std::filesystem::path CreateOrGetDumpDir() {
  std::filesystem::path capture_dir = CreateOrGetOrbitAppDataDir() / "dumps";
  CreateDirectoryOrDie(capture_dir);
  return capture_dir;
}

std::filesystem::path CreateOrGetOrbitAppDataDir() {
#ifdef WIN32
  std::filesystem::path path = std::filesystem::path(GetEnvVar("APPDATA")) / "OrbitProfiler";
#else
  std::filesystem::path path = std::filesystem::path(GetEnvVar("HOME")) / ".orbitprofiler";
#endif
  CreateDirectoryOrDie(path);
  return path;
}

static std::filesystem::path GetDocumentsPath() {
#ifdef WIN32
  PWSTR ppszPath;
  HRESULT result = SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &ppszPath);

  if (result != S_OK) {
    CoTaskMemFree(ppszPath);
    LPSTR error_string = nullptr;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&error_string), 0, nullptr);
    std::filesystem::path path = std::filesystem::path(GetEnvVar("USERPROFILE")) / "Documents";
    ERROR("Retrieving path to Documents (defaulting to \"%s\"): %s", path.string(), error_string);
    LocalFree(error_string);
    return path;
  }

  std::wstring wpath = ppszPath;
  CoTaskMemFree(ppszPath);
  std::filesystem::path path{wpath};
  LOG("Path to Documents: %s", path.string());
  return path;
#else
  return std::filesystem::path(GetEnvVar("HOME")) / "Documents";
#endif
}

std::filesystem::path CreateOrGetOrbitUserDataDir() {
  std::filesystem::path path = GetDocumentsPath() / "Orbit";
  CreateDirectoryOrDie(path);
  return path;
}

std::filesystem::path CreateOrGetLogDir() {
  std::filesystem::path logs_dir;
  if (!absl::GetFlag(FLAGS_log_dir).empty()) {
    logs_dir = absl::GetFlag(FLAGS_log_dir);
  } else {
    logs_dir = CreateOrGetOrbitAppDataDir() / "logs";
  }
  CreateDirectoryOrDie(logs_dir);
  return logs_dir;
}

std::filesystem::path GetLogFilePath() {
  return CreateOrGetLogDir() / orbit_base::GetLogFileName();
}

}  // namespace orbit_core