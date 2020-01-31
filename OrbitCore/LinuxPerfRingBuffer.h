//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------

#ifndef ORBIT_CORE_PERF_RING_BUFFER_H_
#define ORBIT_CORE_PERF_RING_BUFFER_H_

#include <linux/perf_event.h>

#include <cassert>

#include "LinuxPerfUtils.h"

class LinuxPerfRingBuffer {
 public:
  explicit LinuxPerfRingBuffer(uint32_t a_PerfFileDescriptor);
  ~LinuxPerfRingBuffer();

  LinuxPerfRingBuffer(LinuxPerfRingBuffer&&) noexcept;
  LinuxPerfRingBuffer& operator=(LinuxPerfRingBuffer&&) noexcept;

  LinuxPerfRingBuffer(const LinuxPerfRingBuffer&) = delete;
  LinuxPerfRingBuffer& operator=(const LinuxPerfRingBuffer&) = delete;

  bool HasNewData();
  void ReadHeader(perf_event_header* a_Header);
  void SkipRecord(const perf_event_header& a_Header);

  uint32_t FileDescriptor() const { return m_FileDescriptor; }

  template <typename LinuxPerfEvent>
  LinuxPerfEvent ConsumeRecord(const perf_event_header& a_Header) {
    LinuxPerfEvent record;

    // perf_event_header::size contains the size of the entire record.
    // This must be the same as the size of the ring_buffer_data field,
    // in which we want to copy the data.
    // If the sizes are not the same, the memory layout defined in
    // LinuxPerfEvent.h does not match the one found in the ring buffer.
    assert(sizeof(record.ring_buffer_data) == a_Header.size &&
           "Incorrect layout of the perf ring buffer data.");

    // Copy the data from the ringbuffer into the placeholer in that class.
    auto* dest = reinterpret_cast<uint8_t*>(&record.ring_buffer_data);
    Read(dest, a_Header.size);

    SkipRecord(a_Header);

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

  int32_t m_FileDescriptor = -1;
  perf_event_mmap_page* m_Metadata = nullptr;
  char* m_Buffer = nullptr;
  uint64_t m_BufferLength = 0;
  // the buffer length must be a power of 2, so we can do shifting for division.
  uint32_t m_BufferLengthExponent = 0;

  void Read(uint8_t* a_Destination, uint64_t a_Count);

  void* mmap_mapping(int32_t a_FileDescriptor);
};

#endif  // ORBIT_CORE_PERF_RING_BUFFER_H_
