#ifndef ORBIT_LINUX_TRACING_UTILS_H_
#define ORBIT_LINUX_TRACING_UTILS_H_

#include <OrbitBase/Logging.h>

#include <fstream>
#include <thread>

#include "absl/strings/str_format.h"
#include "absl/strings/str_split.h"

namespace LinuxTracing {

std::optional<std::string> ReadFile(std::string_view filename) {
  std::ifstream file(std::string{filename}, std::ios::in | std::ios::binary);
  if (!file) {
    ERROR("Could not open \"%s\"", std::string{filename}.c_str());
    return std::optional<std::string>{};
  }

  std::ostringstream content;
  content << file.rdbuf();
  return content.str();
}

std::string ReadMaps(pid_t pid) {
  std::string maps_filename = absl::StrFormat("/proc/%d/maps", pid);
  std::optional<std::string> maps_content_opt = ReadFile(maps_filename);
  if (maps_content_opt.has_value()) {
    return maps_content_opt.value();
  } else {
    return "";
  }
}

std::string ExecuteCommand(const std::string& cmd) {
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

std::vector<pid_t> ListThreads(pid_t pid) {
  std::vector<pid_t> threads;
  std::string result = ExecuteCommand(absl::StrFormat("ls /proc/%d/task", pid));

  std::stringstream ss(result);
  std::string line;
  while (std::getline(ss, line, '\n')) {
    threads.push_back(std::stol(line));
  }

  return threads;
}

int GetNumCores() {
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

// Read /proc/<pid>/cgroup.
static std::optional<std::string> ReadCgroupContent(pid_t pid) {
  std::string cgroup_filename = absl::StrFormat("/proc/%d/cgroup", pid);
  return ReadFile(cgroup_filename);
}

// Extract the cpuset entry from the content of /proc/<pid>/cgroup.
std::optional<std::string> ExtractCpusetFromCgroup(
    const std::string& cgroup_content) {
  std::istringstream cgroup_content_ss{cgroup_content};
  std::string cgroup_line;
  while (std::getline(cgroup_content_ss, cgroup_line)) {
    if (cgroup_line.find("cpuset:") != std::string::npos ||
        cgroup_line.find("cpuset,") != std::string::npos) {
      // For example "8:cpuset:/" or "8:cpuset:/game", but potentially also
      // "5:cpuacct,cpu,cpuset:/daemons".
      return cgroup_line.substr(cgroup_line.find_last_of(':') + 1);
    }
  }

  return std::optional<std::string>{};
}

// Read /sys/fs/cgroup/cpuset/<cgroup>/cpuset.cpus.
static std::optional<std::string> ReadCpusetCpusContent(
    const std::string& cgroup_cpuset) {
  std::string cpuset_cpus_filename =
      absl::StrFormat("/sys/fs/cgroup/cpuset%s/cpuset.cpus",
                      cgroup_cpuset == "/" ? "" : cgroup_cpuset);
  return ReadFile(cpuset_cpus_filename);
}

std::vector<int> ParseCpusetCpus(const std::string& cpuset_cpus_content) {
  std::vector<int> cpuset_cpus{};
  // Example of format: "0-2,7,12-14".
  for (const auto& range :
       absl::StrSplit(cpuset_cpus_content, ',', absl::SkipEmpty())) {
    std::vector<std::string> values = absl::StrSplit(range, '-');
    if (values.size() == 1) {
      int cpu = std::stoi(values[0]);
      cpuset_cpus.push_back(cpu);
    } else if (values.size() == 2) {
      for (int cpu = std::stoi(values[0]); cpu <= std::stoi(values[1]); ++cpu) {
        cpuset_cpus.push_back(cpu);
      }
    }
  }
  return cpuset_cpus;
}

// Read and parse /sys/fs/cgroup/cpuset/<cgroup_cpuset>/cpuset.cpus for the
// cgroup cpuset of the process with this pid.
// An empty result indicates an error, as trying to start a process with an
// empty cpuset fails with message "cgroup change of group failed".
std::vector<int> GetCpusetCpus(pid_t pid) {
  std::optional<std::string> cgroup_content_opt = ReadCgroupContent(pid);
  if (!cgroup_content_opt.has_value()) {
    return {};
  }

  // For example "/" or "/game".
  std::optional<std::string> cgroup_cpuset_opt =
      ExtractCpusetFromCgroup(cgroup_content_opt.value());
  if (!cgroup_cpuset_opt.has_value()) {
    return {};
  }

  // For example "0-2,7,12-14".
  std::optional<std::string> cpuset_cpus_content_opt =
      ReadCpusetCpusContent(cgroup_cpuset_opt.value());
  if (!cpuset_cpus_content_opt.has_value()) {
    return {};
  }

  return ParseCpusetCpus(cpuset_cpus_content_opt.value());
}

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_UTILS_H_
