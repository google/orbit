/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <procinfo/process.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>

#include <android-base/unique_fd.h>

using android::base::unique_fd;

namespace android {
namespace procinfo {

bool GetProcessInfo(pid_t tid, ProcessInfo* process_info, std::string* error) {
  char path[PATH_MAX];
  snprintf(path, sizeof(path), "/proc/%d", tid);

  unique_fd dirfd(open(path, O_DIRECTORY | O_RDONLY));
  if (dirfd == -1) {
    if (error != nullptr) {
      *error = std::string("failed to open ") + path;
    }
    return false;
  }

  return GetProcessInfoFromProcPidFd(dirfd.get(), process_info, error);
}

static ProcessState parse_state(char state) {
  switch (state) {
    case 'R':
      return kProcessStateRunning;
    case 'S':
      return kProcessStateSleeping;
    case 'D':
      return kProcessStateUninterruptibleWait;
    case 'T':
      return kProcessStateStopped;
    case 'Z':
      return kProcessStateZombie;
    default:
      return kProcessStateUnknown;
  }
}

bool GetProcessInfoFromProcPidFd(int fd, ProcessInfo* process_info,
                                 std::string* error /* can be nullptr */) {
  int status_fd = openat(fd, "status", O_RDONLY | O_CLOEXEC);

  auto set_error = [&error](const char* err) {
    if (error != nullptr) {
      *error = err;
    }
  };

  if (status_fd == -1) {
    set_error("failed to open status fd in GetProcessInfoFromProcPidFd");
    return false;
  }

  std::unique_ptr<FILE, decltype(&fclose)> fp(fdopen(status_fd, "r"), fclose);
  if (!fp) {
    set_error("failed to open status file in GetProcessInfoFromProcPidFd");
    close(status_fd);
    return false;
  }

  int field_bitmap = 0;
  static constexpr int finished_bitmap = 63;
  char* line = nullptr;
  size_t len = 0;

  while (getline(&line, &len, fp.get()) != -1 && field_bitmap != finished_bitmap) {
    char* tab = strchr(line, '\t');
    if (tab == nullptr) {
      continue;
    }

    size_t header_len = tab - line;
    std::string header = std::string(line, header_len);
    if (header == "Name:") {
      std::string name = line + header_len + 1;

      // line includes the trailing newline.
      name.pop_back();
      process_info->name = std::move(name);

      field_bitmap |= 1;
    }  else if (header == "Tgid:") {
      process_info->pid = atoi(tab + 1);
      field_bitmap |= 2;
    } else if (header == "Pid:") {
      process_info->tid = atoi(tab + 1);
      field_bitmap |= 4;
    } else if (header == "TracerPid:") {
      process_info->tracer = atoi(tab + 1);
      field_bitmap |= 8;
    } else if (header == "Uid:") {
      process_info->uid = atoi(tab + 1);
      field_bitmap |= 16;
    } else if (header == "Gid:") {
      process_info->gid = atoi(tab + 1);
      field_bitmap |= 32;
    }
  }

  free(line);
  if (field_bitmap != finished_bitmap) {
    set_error("failed to parse /proc/<pid>/status");
    return false;
  }

  unique_fd stat_fd(openat(fd, "stat", O_RDONLY | O_CLOEXEC));
  if (stat_fd == -1) {
    set_error("failed to open /proc/<pid>/stat");
  }

  std::string stat;
  if (!android::base::ReadFdToString(stat_fd, &stat)) {
    set_error("failed to read /proc/<pid>/stat");
    return false;
  }

  // See man 5 proc. There's no reason comm can't contain ' ' or ')',
  // so we search backwards for the end of it.
  const char* end_of_comm = strrchr(stat.c_str(), ')');

  static constexpr const char* pattern =
      "%c "    // state
      "%d "    // ppid
      "%*d "   // pgrp
      "%*d "   // session
      "%*d "   // tty_nr
      "%*d "   // tpgid
      "%*u "   // flags
      "%*lu "  // minflt
      "%*lu "  // cminflt
      "%*lu "  // majflt
      "%*lu "  // cmajflt
      "%*lu "  // utime
      "%*lu "  // stime
      "%*ld "  // cutime
      "%*ld "  // cstime
      "%*ld "  // priority
      "%*ld "  // nice
      "%*ld "  // num_threads
      "%*ld "  // itrealvalue
      "%llu "  // starttime
      ;

  char state = '\0';
  int ppid = 0;
  unsigned long long start_time = 0;
  int rc = sscanf(end_of_comm + 2, pattern, &state, &ppid, &start_time);
  if (rc != 3) {
    set_error("failed to parse /proc/<pid>/stat");
    return false;
  }

  process_info->state = parse_state(state);
  process_info->ppid = ppid;
  process_info->starttime = start_time;
  return true;
}

} /* namespace procinfo */
} /* namespace android */
