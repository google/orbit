// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitPaths/Paths.h"

#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>
#include <absl/strings/string_view.h>
#include <stdlib.h>

#include <filesystem>
#include <optional>
#include <string>

#include "OrbitBase/Result.h"

#ifdef _WIN32
// clang-format off
#include <Windows.h>
#include <KnownFolders.h>
// clang-format on
#include <shlobj.h>

#include "OrbitBase/StringConversion.h"
#endif

#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"

ABSL_FLAG(std::string, log_dir, "", "Set directory for the log.");

constexpr std::string_view kOrbitFolderInDocumentsName{"Orbit"};
constexpr std::string_view kCapturesFolderName{"captures"};
constexpr std::string_view kPresetsFolderName{"presets"};
constexpr std::string_view kCacheFolderName{"cache"};
constexpr std::string_view kDumpsFolderName{"dumps"};
constexpr std::string_view kLogsFolderName{"logs"};
constexpr std::string_view kConfigFolderName{"config"};
constexpr std::string_view kSymbolPathsFileName{"SymbolPaths.txt"};

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
// successful or it already existed. If an error occurs its logged and returned. The difference to
// orbit_base::CreateDirectories is the return type and logging.
static ErrorMessageOr<void> CreateDirectoryIfItDoesNotExist(
    const std::filesystem::path& directory) {
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

static ErrorMessageOr<std::filesystem::path> CreateAndGetConfigPath() {
  OUTCOME_TRY(std::filesystem::path app_data_dir, CreateOrGetOrbitAppDataDir());
  std::filesystem::path config_dir = app_data_dir / kConfigFolderName;
  OUTCOME_TRY(CreateDirectoryIfItDoesNotExist(config_dir));
  return config_dir;
}

ErrorMessageOr<std::filesystem::path> GetSymbolsFilePath() {
  OUTCOME_TRY(std::filesystem::path config_dir, CreateAndGetConfigPath());
  return config_dir / kSymbolPathsFileName;
}

std::filesystem::path CreateOrGetCacheDirUnsafe() {
  std::filesystem::path cache_dir = CreateOrGetOrbitAppDataDirUnsafe() / kCacheFolderName;
  CreateDirectoryOrDie(cache_dir);
  return cache_dir;
}

ErrorMessageOr<std::filesystem::path> CreateOrGetCacheDir() {
  OUTCOME_TRY(std::filesystem::path app_data_dir, CreateOrGetOrbitAppDataDir());
  std::filesystem::path cache_dir = app_data_dir / kCacheFolderName;
  OUTCOME_TRY(CreateDirectoryIfItDoesNotExist(cache_dir));
  return cache_dir;
}

std::filesystem::path CreateOrGetPresetDirUnsafe() {
  std::filesystem::path preset_dir = CreateOrGetOrbitUserDataDirUnsafe() / kPresetsFolderName;
  CreateDirectoryOrDie(preset_dir);
  return preset_dir;
}

ErrorMessageOr<std::filesystem::path> CreateOrGetPresetDir() {
  OUTCOME_TRY(std::filesystem::path user_data_dir, CreateOrGetOrbitUserDataDir());
  std::filesystem::path preset_dir = user_data_dir / kPresetsFolderName;
  OUTCOME_TRY(CreateDirectoryIfItDoesNotExist(preset_dir));
  return preset_dir;
}

std::filesystem::path CreateOrGetCaptureDirUnsafe() {
  std::filesystem::path capture_dir = CreateOrGetOrbitUserDataDirUnsafe() / kCapturesFolderName;
  CreateDirectoryOrDie(capture_dir);
  return capture_dir;
}

ErrorMessageOr<std::filesystem::path> CreateOrGetCaptureDir() {
  OUTCOME_TRY(std::filesystem::path user_data_dir, CreateOrGetOrbitUserDataDir());
  std::filesystem::path capture_dir = user_data_dir / kCapturesFolderName;
  OUTCOME_TRY(CreateDirectoryIfItDoesNotExist(capture_dir));
  return capture_dir;
}

std::filesystem::path CreateOrGetDumpDirUnsafe() {
  std::filesystem::path dumps_dir = CreateOrGetOrbitAppDataDirUnsafe() / kDumpsFolderName;
  CreateDirectoryOrDie(dumps_dir);
  return dumps_dir;
}

ErrorMessageOr<std::filesystem::path> CreateOrGetDumpDir() {
  OUTCOME_TRY(std::filesystem::path app_data_dir, CreateOrGetOrbitAppDataDir());
  std::filesystem::path dumps_dir = app_data_dir / kDumpsFolderName;
  OUTCOME_TRY(CreateDirectoryIfItDoesNotExist(dumps_dir));
  return dumps_dir;
}

static std::filesystem::path GetOrbitAppDataDir() {
#ifdef _WIN32
  std::filesystem::path path = std::filesystem::path(GetEnvVar("APPDATA")) / "OrbitProfiler";
#else
  std::filesystem::path path = std::filesystem::path(GetEnvVar("HOME")) / ".orbitprofiler";
#endif
  return path;
}

std::filesystem::path CreateOrGetOrbitAppDataDirUnsafe() {
  std::filesystem::path path = GetOrbitAppDataDir();
  CreateDirectoryOrDie(path);
  return path;
}

ErrorMessageOr<std::filesystem::path> CreateOrGetOrbitAppDataDir() {
  std::filesystem::path path = GetOrbitAppDataDir();
  OUTCOME_TRY(CreateDirectoryIfItDoesNotExist(path));
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
  ORBIT_LOG("Path to Documents: %s", orbit_base::ToStdString(path.wstring()));
  return path;
#else
  return std::filesystem::path(GetEnvVar("HOME")) / "Documents";
#endif
}

std::filesystem::path CreateOrGetOrbitUserDataDirUnsafe() {
  std::filesystem::path path = GetDocumentsPath() / kOrbitFolderInDocumentsName;
  CreateDirectoryOrDie(path);
  return path;
}

ErrorMessageOr<std::filesystem::path> CreateOrGetOrbitUserDataDir() {
  std::filesystem::path path = GetDocumentsPath() / kOrbitFolderInDocumentsName;
  OUTCOME_TRY(CreateDirectoryIfItDoesNotExist(path));
  return path;
}

static std::optional<std::filesystem::path> GetLogDirFromFlag() {
  std::filesystem::path logs_dir;
  if (!absl::GetFlag(FLAGS_log_dir).empty()) {
    return std::filesystem::path{absl::GetFlag(FLAGS_log_dir)};
  }
  return std::nullopt;
}

std::filesystem::path CreateOrGetLogDirUnsafe() {
  std::filesystem::path logs_dir =
      GetLogDirFromFlag().value_or(CreateOrGetOrbitAppDataDirUnsafe() / kLogsFolderName);
  CreateDirectoryOrDie(logs_dir);
  return logs_dir;
}

ErrorMessageOr<std::filesystem::path> CreateOrGetLogDir() {
  OUTCOME_TRY(std::filesystem::path app_data_dir, CreateOrGetOrbitAppDataDir());
  std::filesystem::path logs_dir = GetLogDirFromFlag().value_or(app_data_dir / kLogsFolderName);
  OUTCOME_TRY(CreateDirectoryIfItDoesNotExist(logs_dir));
  return logs_dir;
}

std::filesystem::path GetLogFilePathUnsafe() {
  return CreateOrGetLogDirUnsafe() / orbit_base::GetLogFileName();
}

ErrorMessageOr<std::filesystem::path> GetLogFilePath() {
  OUTCOME_TRY(std::filesystem::path log_dir, CreateOrGetLogDir());
  return log_dir / orbit_base::GetLogFileName();
}

}  // namespace orbit_paths