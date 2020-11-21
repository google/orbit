// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// clang-format off
#include <OrbitBase/Platform/Linux/Profiling.h>
// clang-format on

#include <OrbitBase/Logging.h>

#include <fstream>
#include <optional>

namespace {
static std::optional<std::string> ReadFile(std::string_view filename) {
  std::ifstream file{std::string{filename}, std::ios::in | std::ios::binary};
  if (!file) {
    ERROR("Could not open \"%s\"", std::string{filename}.c_str());
    return std::optional<std::string>{};
  }

  std::ostringstream content;
  content << file.rdbuf();
  return content.str();
}
}  // namespace

std::string GetThreadName(pid_t tid) {
  std::string comm_filename = absl::StrFormat("/proc/%d/comm", tid);
  std::optional<std::string> comm_content = ReadFile(comm_filename);
  if (!comm_content.has_value()) {
    const std::string kEmptyString;
    return kEmptyString;
  }
  if (comm_content.value().back() == '\n') {
    comm_content.value().pop_back();
  }
  return comm_content.value();
}

void SetThreadName(const std::string& thread_name) {
  // On Linux, "the thread name is a meaningful C language
  // string, whose length is restricted to 16 characters,
  // including the terminating null byte ('\0')".
  constexpr size_t kMaxStringSize = 16;
  std::string name = thread_name;
  if (name.length() >= kMaxStringSize) {
    name[kMaxStringSize - 1] = 0;
  }
  pthread_setname_np(pthread_self(), name.data());
}
