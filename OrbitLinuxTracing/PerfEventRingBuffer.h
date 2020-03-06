#ifndef ORBIT_LINUX_TRACING_PERF_RING_BUFFER_H_
#define ORBIT_LINUX_TRACING_PERF_RING_BUFFER_H_

#include <linux/perf_event.h>

#include <cassert>

#include "PerfEventOpen.h"

namespace LinuxTracing {

class PerfEventRingBuffer {
 public:
  explicit PerfEventRingBuffer(int perf_event_fd, uint64_t size_kb);
  ~PerfEventRingBuffer();

  PerfEventRingBuffer(PerfEventRingBuffer&&) noexcept;
  PerfEventRingBuffer& operator=(PerfEventRingBuffer&&) noexcept;

  PerfEventRingBuffer(const PerfEventRingBuffer&) = delete;
  PerfEventRingBuffer& operator=(const PerfEventRingBuffer&) = delete;

  bool IsOpen() const { return ring_buffer_ != nullptr; }

  bool HasNewData();
  void ReadHeader(perf_event_header* header);
  void SkipRecord(const perf_event_header& header);
  void ConsumeRecord(const perf_event_header& header, void* record);

 private:
  uint64_t mmap_length_ = 0;
  perf_event_mmap_page* metadata_page_ = nullptr;
  char* ring_buffer_ = nullptr;
  uint64_t ring_buffer_size_ = 0;
  // The buffer length needs to be a power of 2, hence we can use shifting for
  // division.
  uint32_t ring_buffer_size_log2_ = 0;

  void ReadAtTail(uint8_t* dest, uint64_t count);
  void ReadAtOffsetFromTail(uint8_t* dest, uint64_t offset_from_tail,
                            uint64_t count);
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_PERF_RING_BUFFER_H_
