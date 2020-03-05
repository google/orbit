#ifndef ORBIT_LINUX_TRACING_UTILS_H_
#define ORBIT_LINUX_TRACING_UTILS_H_

#include <unistd.h>

#include <fstream>
#include <thread>

#include "Logging.h"
#include "absl/strings/str_format.h"

namespace LinuxTracing {

inline uint64_t MonotonicTimestampNs() {
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return 1'000'000'000llu * ts.tv_sec + ts.tv_nsec;
}

inline std::string ExecuteCommand(const std::string& cmd) {
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"),
                                                pclose);
  if (!pipe) {
    ERROR("Could not open pipe for \"%s\"", cmd.c_str());
  }

  std::array<char, 128> buffer;
  std::string result;
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

inline std::string ReadMaps(pid_t pid) {
  std::string maps_filename = absl::StrFormat("/proc/%d/maps", pid);
  std::ifstream maps_file{maps_filename};
  if (!maps_file) {
    ERROR("Could not open \"%s\"", maps_filename.c_str());
    return "";
  }

  std::string maps_buffer;
  std::string maps_line;
  while (std::getline(maps_file, maps_line)) {
    maps_buffer.append(maps_line).append("\n");
  }
  return maps_buffer;
}

inline std::vector<pid_t> ListThreads(pid_t pid) {
  std::vector<pid_t> threads;
  std::string result = ExecuteCommand(absl::StrFormat("ls /proc/%d/task", pid));

  std::stringstream ss(result);
  std::string line;
  while (std::getline(ss, line, '\n')) {
    threads.push_back(std::stol(line));
  }

  return threads;
}

inline int GetNumCores() {
  int num_cores = static_cast<int>(std::thread::hardware_concurrency());
  // Some compilers do not support std::thread::hardware_concurrency().
  if (num_cores != 0) {
    return num_cores;
  }

  std::string num_cores_str = ExecuteCommand("nproc");
  if (!num_cores_str.empty()) {
    return std::stoi(num_cores_str);
  }

  return 1;
}

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_UTILS_H_
