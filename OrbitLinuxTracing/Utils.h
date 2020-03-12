#ifndef ORBIT_LINUX_TRACING_UTILS_H_
#define ORBIT_LINUX_TRACING_UTILS_H_

#include <unistd.h>

#include <optional>

namespace LinuxTracing {

inline uint64_t MonotonicTimestampNs() {
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return 1'000'000'000llu * ts.tv_sec + ts.tv_nsec;
}

std::string ExecuteCommand(const std::string& cmd);

std::optional<std::string> ReadFile(std::string_view filename);

std::string ReadMaps(pid_t pid);

std::vector<pid_t> ListThreads(pid_t pid);

int GetNumCores();

std::vector<int> GetCpusetCpus(pid_t pid);

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_UTILS_H_
