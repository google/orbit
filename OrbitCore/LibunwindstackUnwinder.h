#ifndef ORBIT_CORE_LIBUNWINDSTACK_UNWINDER_H_
#define ORBIT_CORE_LIBUNWINDSTACK_UNWINDER_H_

#include <asm/perf_regs.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "unwindstack/MachineX86_64.h"
#include "unwindstack/RegsX86_64.h"
#include "unwindstack/Unwinder.h"

class LibunwindstackUnwinder {
 public:
  bool SetMaps(const std::string& maps_buffer);

  std::vector<unwindstack::FrameData> Unwind(
      const std::array<uint64_t, PERF_REG_X86_64_MAX>& perf_regs,
      const char* stack_dump, uint64_t stack_dump_size);

 private:
  std::unique_ptr<unwindstack::BufferMaps> maps_{nullptr};

  static constexpr size_t MAX_FRAMES = 1024;  // This is arbitrary.

  static const std::array<int, unwindstack::X86_64_REG_LAST>
      UNWINDSTACK_REGS_TO_PERF_REGS;

  static std::string LibunwindstackErrorString(unwindstack::ErrorCode error_code) {
    static const std::vector<const char*> ERROR_NAMES{
        "ERROR_NONE",
        "ERROR_MEMORY_INVALID",
        "ERROR_UNWIND_INFO",
        "ERROR_UNSUPPORTED",
        "ERROR_INVALID_MAP",
        "ERROR_MAX_FRAMES_EXCEEDED",
        "ERROR_REPEATED_FRAME",
        "ERROR_INVALID_ELF"};
    return ERROR_NAMES[error_code];
  }
};

#endif  // ORBIT_CORE_LIBUNWINDSTACK_UNWINDER_H_