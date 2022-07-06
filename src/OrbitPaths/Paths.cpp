// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitPaths/Paths.h"

#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>

#include <filesystem>
#include <string>

#include "OrbitBase/Result.h"

#ifdef _WIN32
// clang-format off
#include <Windows.h>
#include <KnownFolders.h>
// clang-format on
#include <shlobj.h>
#endif

#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"

ABSL_FLAG(std::string, log_dir, "", "Set directory for the log.");

constexpr std::string_view kOrbitFolderName{"Orbit"};
constexpr std::string_view kCapturesFolderName{"captures"};
constexpr std::string_view kPresetsFolderName{"presets"};

namespace orbit_paths {

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

// Attempts to create a directory if it doesn't exist. Returns success if the creation was
// successful or it already existed. If an error occurs its logged and returned.
static ErrorMessageOr<void> CreateDirectoryIfNecessary(const std::filesystem::path& directory) {
  ErrorMessageOr<bool> created_or_error = orbit_base::CreateDirectories(directory);
  if (created_or_error.has_error()) {
    std::string error_message =
        absl::StrFormat("Unable to create directory %s: %s", directory.string(),
                        created_or_error.error().message());
    ORBIT_ERROR("%s", error_message);
    return ErrorMessage(error_message);
  }
  return outcome::success();
}

static void CreateDirectoryOrDie(const std::filesystem::path& directory) {
  auto create_directories_result = orbit_base::CreateDirectories(directory);
  if (create_directories_result.has_error()) {
    ORBIT_FATAL("Unable to create directory \"%s\": %s", directory.string(),
                create_directories_result.error().message());
  }
}

static std::filesystem::path CreateAndGetConfigPath() {
  std::filesystem::path config_dir = CreateOrGetOrbitAppDataDir() / "config";
  CreateDirectoryOrDie(config_dir);
  return config_dir;
}

std::filesystem::path GetSymbolsFilePath() { return CreateAndGetConfigPath() / "SymbolPaths.txt"; }

std::filesystem::path CreateOrGetCacheDir() {
  std::filesystem::path cache_dir = CreateOrGetOrbitAppDataDir() / "cache";
  CreateDirectoryOrDie(cache_dir);
  return cache_dir;
}

std::filesystem::path GetPresetDirPriorTo1_66() { return CreateOrGetOrbitAppDataDir() / "presets"; }

std::filesystem::path GetCaptureDirPriorTo1_66() { return CreateOrGetOrbitAppDataDir() / "output"; }

std::filesystem::path CreateOrGetPresetDir() {
  std::filesystem::path preset_dir = CreateOrGetOrbitUserDataDir() / kPresetsFolderName;
  CreateDirectoryOrDie(preset_dir);
  return preset_dir;
}

ErrorMessageOr<std::filesystem::path> CreateOrGetPresetDirSafe() {
  OUTCOME_TRY(std::filesystem::path user_data_dir, CreateOrGetOrbitUserDataDirSafe());
  std::filesystem::path preset_dir = user_data_dir / kPresetsFolderName;
  OUTCOME_TRY(CreateDirectoryIfNecessary(preset_dir));
  return preset_dir;
}

std::filesystem::path CreateOrGetCaptureDir() {
  std::filesystem::path capture_dir = CreateOrGetOrbitUserDataDir() / kCapturesFolderName;
  CreateDirectoryOrDie(capture_dir);
  return capture_dir;
}

ErrorMessageOr<std::filesystem::path> CreateOrGetCaptureDirSafe() {
  OUTCOME_TRY(std::filesystem::path user_data_dir, CreateOrGetOrbitUserDataDirSafe());
  std::filesystem::path capture_dir = user_data_dir / kCapturesFolderName;
  OUTCOME_TRY(CreateDirectoryIfNecessary(capture_dir));
  return capture_dir;
}

std::filesystem::path CreateOrGetDumpDir() {
  std::filesystem::path capture_dir = CreateOrGetOrbitAppDataDir() / "dumps";
  CreateDirectoryOrDie(capture_dir);
  return capture_dir;
}

static std::filesystem::path GetOrbitAppDataDir() {
#ifdef _WIN32
  std::filesystem::path path = std::filesystem::path(GetEnvVar("APPDATA")) / "OrbitProfiler";
#else
  std::filesystem::path path = std::filesystem::path(GetEnvVar("HOME")) / ".orbitprofiler";
#endif
  return path;
}

std::filesystem::path CreateOrGetOrbitAppDataDir() {
  std::filesystem::path path = GetOrbitAppDataDir();
  CreateDirectoryOrDie(path);
  return path;
}

ErrorMessageOr<std::filesystem::path> CreateOrGetOrbitAppDataDirSafe() {
  std::filesystem::path path = GetOrbitAppDataDir();
  OUTCOME_TRY(CreateDirectoryIfNecessary(path));
  return path;
}

static std::filesystem::path GetDocumentsPath() {
#ifdef _WIN32
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
    ORBIT_ERROR("Retrieving path to Documents (defaulting to \"%s\"): %s", path.string(),
                error_string);
    LocalFree(error_string);
    return path;
  }

  std::wstring wpath = ppszPath;
  CoTaskMemFree(ppszPath);
  std::filesystem::path path{wpath};
  ORBIT_LOG("Path to Documents: %s", path.string());
  return path;
#else
  return std::filesystem::path(GetEnvVar("HOME")) / "Documents";
#endif
}

std::filesystem::path CreateOrGetOrbitUserDataDir() {
  std::filesystem::path path = GetDocumentsPath() / kOrbitFolderName;
  CreateDirectoryOrDie(path);
  return path;
}

ErrorMessageOr<std::filesystem::path> CreateOrGetOrbitUserDataDirSafe() {
  std::filesystem::path path = GetDocumentsPath() / kOrbitFolderName;
  OUTCOME_TRY(CreateDirectoryIfNecessary(path));
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

}  // namespace orbit_paths