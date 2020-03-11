#ifndef ORBIT_LINUX_TRACING_UTILS_H_
#define ORBIT_LINUX_TRACING_UTILS_H_

#include <unistd.h>

#include <fstream>
#include <optional>
#include <thread>

#include "Logging.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_split.h"

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

inline std::optional<std::string> ReadFile(std::string_view filename) {
  std::ifstream file(std::string{filename}, std::ios::in | std::ios::binary);
  if (!file) {
    ERROR("Could not open \"%s\"", std::string{filename}.c_str());
    return std::optional<std::string>{};
  }

  std::ostringstream content;
  content << file.rdbuf();
  return content.str();
}

inline std::string ReadMaps(pid_t pid) {
  std::string maps_filename = absl::StrFormat("/proc/%d/maps", pid);
  std::optional<std::string> maps_content_opt = ReadFile(maps_filename);
  if (maps_content_opt.has_value()) {
    return maps_content_opt.value();
  } else {
    return "";
  }
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

// Read the cpuset entry in /proc/<pid>/cgroup.
inline std::optional<std::string> GetCgroupCpuset(pid_t pid) {
  std::string cgroup_filename = absl::StrFormat("/proc/%d/cgroup", pid);
  std::optional<std::string> cgroup_content_opt = ReadFile(cgroup_filename);

  if (!cgroup_content_opt.has_value()) {
    return std::optional<std::string>{};
  }
  std::istringstream cgroup_content{cgroup_content_opt.value()};

  static const std::string CPUSET_SUBSTR = ":cpuset:";
  std::string cgroup_line;
  while (std::getline(cgroup_content, cgroup_line)) {
    size_t cpuset_substr_pos = cgroup_line.find(CPUSET_SUBSTR);
    if (cpuset_substr_pos != std::string::npos) {
      // For example "8:cpuset:/" or "8:cpuset:/game".
      return cgroup_line.substr(cpuset_substr_pos + CPUSET_SUBSTR.size());
    }
  }

  return std::optional<std::string>{};
}

// Read and parse /sys/fs/cgroup/cpuset/<cgroup_cpuset>/cpuset.cpus for the
// cgroup cpuset of the process with this pid.
inline std::vector<int> GetCpusetCpus(pid_t pid) {
  std::optional<std::string> cgroup_cpuset_opt = GetCgroupCpuset(pid);
  if (!cgroup_cpuset_opt.has_value()) {
    return {};
  }

  // For example "/" or "/game".
  const std::string& cgroup_cpuset = cgroup_cpuset_opt.value();

  std::string cpuset_cpus_filename =
      absl::StrFormat("/sys/fs/cgroup/cpuset%s/cpuset.cpus",
                      cgroup_cpuset == "/" ? "" : cgroup_cpuset);
  std::optional<std::string> cpuset_cpus_content_opt =
      ReadFile(cpuset_cpus_filename);
  if (!cpuset_cpus_content_opt.has_value()) {
    return {};
  }

  std::string cpuset_cpus_content = cpuset_cpus_content_opt.value();
  std::vector<int> cpuset_cpus{};
  // Example of format: 0-2,7,12-14
  for (const auto& range : absl::StrSplit(cpuset_cpus_content, ',')) {
    std::vector<std::string> values = absl::StrSplit(range, '-');
    if (values.size() == 1) {
      int cpu = std::stoi(values[0]);
      cpuset_cpus.push_back(cpu);
    } else if (values.size() == 2) {
      for (int cpu = std::stoi(values[0]); cpu <= std::stoi(values[1]);
           ++cpu) {
        cpuset_cpus.push_back(cpu);
      }
    }
  }
  return cpuset_cpus;
}

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_UTILS_H_
