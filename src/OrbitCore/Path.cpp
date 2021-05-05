// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Path.h"

#include <filesystem>
#include <string>

#include "CoreUtils.h"
#include "OrbitBase/Logging.h"
#include "absl/flags/flag.h"

ABSL_FLAG(std::string, log_dir, "", "Set directory for the log.");

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

static std::filesystem::path CreateAndGetConfigPath() {
  std::filesystem::path config_dir = Path::CreateOrGetOrbitAppDataDir() / "config";
  std::filesystem::create_directory(config_dir);
  return config_dir;
}

std::filesystem::path Path::GetFileMappingFileName() {
  return CreateAndGetConfigPath() / "FileMapping.txt";
}

std::filesystem::path Path::GetSymbolsFileName() {
  return CreateAndGetConfigPath() / "SymbolPaths.txt";
}

std::filesystem::path Path::CreateOrGetCacheDir() {
  std::filesystem::path cache_dir = Path::CreateOrGetOrbitAppDataDir() / "cache";
  std::filesystem::create_directory(cache_dir);
  return cache_dir;
}

std::filesystem::path Path::CreateOrGetPresetDir() {
  std::filesystem::path preset_dir = Path::CreateOrGetOrbitAppDataDir() / "presets";
  std::filesystem::create_directory(preset_dir);
  return preset_dir;
}

std::filesystem::path Path::CreateOrGetCaptureDir() {
  std::filesystem::path capture_dir = Path::CreateOrGetOrbitAppDataDir() / "output";
  std::filesystem::create_directory(capture_dir);
  return capture_dir;
}

std::filesystem::path Path::CreateOrGetDumpDir() {
  std::filesystem::path capture_dir = Path::CreateOrGetOrbitAppDataDir() / "dumps";
  std::filesystem::create_directory(capture_dir);
  return capture_dir;
}

std::filesystem::path Path::CreateOrGetOrbitAppDataDir() {
#ifdef WIN32
  std::filesystem::path path = std::filesystem::path(GetEnvVar("APPDATA")) / "OrbitProfiler";
#else
  std::filesystem::path path = std::filesystem::path(GetEnvVar("HOME")) / ".orbitprofiler";
#endif
  std::filesystem::create_directory(path);
  return path;
}

std::filesystem::path Path::CreateOrGetLogDir() {
  std::filesystem::path logs_dir;
  if (!absl::GetFlag(FLAGS_log_dir).empty()) {
    logs_dir = absl::GetFlag(FLAGS_log_dir);
  } else {
    logs_dir = Path::CreateOrGetOrbitAppDataDir() / "logs";
  }
  std::filesystem::create_directory(logs_dir);
  return logs_dir;
}

std::filesystem::path Path::GetLogFilePath() {
  return Path::CreateOrGetLogDir() / orbit_base::GetLogFileName();
}
