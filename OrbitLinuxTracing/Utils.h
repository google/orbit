#ifndef ORBIT_LINUX_TRACING_UTILS_H_
#define ORBIT_LINUX_TRACING_UTILS_H_

#include <OrbitBase/Logging.h>
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

std::optional<std::string> ExtractCpusetFromCgroup(
    const std::string& cgroup_content);

std::vector<int> ParseCpusetCpus(const std::string& cpuset_cpus_content);

std::vector<int> GetCpusetCpus(pid_t pid);

#if defined(__x86_64__)

#define READ_ONCE(x) (*(volatile typeof(x)*)&x)
#define WRITE_ONCE(x, v) (*(volatile typeof(x)*)&x) = (v)
#define barrier() asm volatile("" ::: "memory")

#define smp_store_release(p, v) \
  do {                          \
    barrier();                  \
    WRITE_ONCE(*p, v);          \
  } while (0)

#define smp_load_acquire(p)          \
  ({                                 \
    typeof(*p) ___p = READ_ONCE(*p); \
    barrier();                       \
    ___p;                            \
  })

#endif

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_UTILS_H_
