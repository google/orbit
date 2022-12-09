// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_PERF_RING_BUFFER_H_
#define LINUX_TRACING_PERF_RING_BUFFER_H_

#include <linux/perf_event.h>
#include <stdint.h>

#include <string>

#include "OrbitBase/Logging.h"

namespace orbit_linux_tracing {

class PerfEventRingBuffer {
 public:
  explicit PerfEventRingBuffer(int perf_event_fd, uint64_t size_kb, std::string name);
  ~PerfEventRingBuffer();

  PerfEventRingBuffer(PerfEventRingBuffer&&);
  PerfEventRingBuffer& operator=(PerfEventRingBuffer&&);

  PerfEventRingBuffer(const PerfEventRingBuffer&) = delete;
  PerfEventRingBuffer& operator=(const PerfEventRingBuffer&) = delete;

  [[nodiscard]] bool IsOpen() const { return ring_buffer_ != nullptr; }
  [[nodiscard]] int GetFileDescriptor() const { return file_descriptor_; }
  [[nodiscard]] const std::string& GetName() const { return name_; }

  bool HasNewData();
  void ReadHeader(perf_event_header* header);
  void SkipRecord(const perf_event_header& header);

  template <typename T>
  void ConsumeRecord(const perf_event_header& header, T* record) {
    ORBIT_CHECK(header.size == sizeof(T));
    ConsumeRawRecord(header, record);
  }

  template <typename T>
  void ReadValueAtOffset(T* value, uint64_t offset) {
    ReadAtOffsetFromTail(value, offset, sizeof(T));
  }

  void ReadRawAtOffset(void* dest, uint64_t offset, uint64_t count) {
    ReadAtOffsetFromTail(dest, offset, count);
  }

 private:
  uint64_t mmap_length_ = 0;
  perf_event_mmap_page* metadata_page_ = nullptr;
  char* ring_buffer_ = nullptr;
  uint64_t ring_buffer_size_ = 0;
  // The buffer length needs to be a power of 2, hence we can use shifting for
  // division.
  uint32_t ring_buffer_size_log2_ = 0;
  int file_descriptor_ = -1;
  std::string name_;

  // ConsumeRawRecord reads header.size bytes into record buffer and then skips the record.
  void ConsumeRawRecord(const perf_event_header& header, void* record);
  void ReadAtTail(void* dest, uint64_t count) { return ReadAtOffsetFromTail(dest, 0, count); }
  void ReadAtOffsetFromTail(void* dest, uint64_t offset_from_tail, uint64_t count);
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_PERF_RING_BUFFER_H_
