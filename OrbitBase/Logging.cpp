// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <OrbitBase/Logging.h>

std::ofstream log_file;

void InitLogFile(const std::string& path) {
  CHECK(!log_file.is_open());
  log_file.open(path, std::ofstream::out);
}
