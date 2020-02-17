#ifndef ORBIT_LINUX_TRACING_PERF_RING_BUFFER_H_
#define ORBIT_LINUX_TRACING_PERF_RING_BUFFER_H_

#include <linux/perf_event.h>

#include <cassert>

#include "PerfEventUtils.h"

class PerfEventRingBuffer {
 public:
  explicit PerfEventRingBuffer(int perf_event_file_descriptor);
  ~PerfEventRingBuffer();

  PerfEventRingBuffer(PerfEventRingBuffer&&) noexcept;
  PerfEventRingBuffer& operator=(PerfEventRingBuffer&&) noexcept;

  PerfEventRingBuffer(const PerfEventRingBuffer&) = delete;
  PerfEventRingBuffer& operator=(const PerfEventRingBuffer&) = delete;

  bool HasNewData();
  void ReadHeader(perf_event_header* header);
  void SkipRecord(const perf_event_header& header);

  template <typename LinuxPerfEvent>
  LinuxPerfEvent ConsumeRecord(const perf_event_header& header) {
    LinuxPerfEvent record;

    // perf_event_header::size contains the size of the entire record.
    // This must be the same as the size of the ring_buffer_data field,
    // in which we want to copy the data.
    // If the sizes are not the same, the memory layout defined in
    // PerfEvent.h does not match the one found in the ring buffer.
    assert(sizeof(record.ring_buffer_data) == header.size &&
           "Incorrect layout of the perf ring buffer data.");

    // Copy the data from the ringbuffer into the placeholer in that class.
    auto* dest = reinterpret_cast<uint8_t*>(&record.ring_buffer_data);
    Read(dest, header.size);

    SkipRecord(header);

    return record;
  }

 private:
  const size_t RING_BUFFER_PAGE_COUNT = 2048;  // 64: 256 KB; 2048: 8 MB
  const size_t PAGE_SIZE = getpagesize();      // 4 KB
  // "The mmap size should be 1+2^n pages, where the first page is a meta‐
  // data page (struct perf_event_mmap_page) that contains various bits of
  // information such as where the ring-buffer head is."
  // http://man7.org/linux/man-pages/man2/perf_event_open.2.html
  const size_t MMAP_LENGTH = (1 + RING_BUFFER_PAGE_COUNT) * PAGE_SIZE;

  perf_event_mmap_page* metadata_ = nullptr;
  char* ring_buffer_ = nullptr;
  uint64_t ring_buffer_size_ = 0;
  // the buffer length must be a power of 2, so we can do shifting for division.
  uint32_t ring_buffer_size_exponent_ = 0;

  void Read(uint8_t* destination, uint64_t count);

  void* mmap_mapping(int32_t file_descriptor);
};

#endif  // ORBIT_LINUX_TRACING_PERF_RING_BUFFER_H_
