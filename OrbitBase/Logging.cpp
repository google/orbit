// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <OrbitBase/Logging.h>

static absl::Mutex log_file_mutex;
std::ofstream log_file;

void InitLogFile(const std::string& path) {
  absl::MutexLock lock(&log_file_mutex);
  CHECK(!log_file.is_open());
  log_file.open(path, std::ofstream::out);
}

void LogToFile(const std::string& message) {
  absl::MutexLock lock(&log_file_mutex);
  if (log_file.is_open()) {
    log_file << message;
    log_file.flush();
  }
}
