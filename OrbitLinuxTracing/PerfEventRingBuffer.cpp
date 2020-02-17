#include "PerfEventRingBuffer.h"

#include <linux/perf_event.h>
#include <sys/mman.h>

#include <cassert>
#include <cstring>
#include <utility>

#include "Logging.h"
#include "PerfEventUtils.h"

PerfEventRingBuffer::PerfEventRingBuffer(int perf_event_file_descriptor) {
  void* mmap_ret = mmap_mapping(perf_event_file_descriptor);

  // The first page, just before the ring buffer, is the metadata page.
  metadata_ = reinterpret_cast<perf_event_mmap_page*>(mmap_ret);
  ring_buffer_size_ = metadata_->data_size;

  // The buffer length is a power of 2 (otherwise mmap for the perf_event_open
  // ring buffer would have failed), so find the exponent in order to later
  // optimize the code using shifts/masks instead of divs/mods.
  uint32_t exponent = 0;
  uint64_t rest = ring_buffer_size_;
  while (rest >>= 1) ++exponent;
  ring_buffer_size_exponent_ = exponent;

  // beginning of the ring buffer
  ring_buffer_ = reinterpret_cast<char*>(mmap_ret) + metadata_->data_offset;
}

PerfEventRingBuffer::PerfEventRingBuffer(PerfEventRingBuffer&& other) noexcept {
  metadata_ = std::exchange(other.metadata_, nullptr);
  ring_buffer_ = std::exchange(other.ring_buffer_, nullptr);
  ring_buffer_size_ = std::exchange(other.ring_buffer_size_, 0);
  ring_buffer_size_exponent_ =
      std::exchange(other.ring_buffer_size_exponent_, 0);
}

PerfEventRingBuffer& PerfEventRingBuffer::operator=(
    PerfEventRingBuffer&& other) noexcept {
  if (&other != this) {
    std::swap(metadata_, other.metadata_);
    std::swap(ring_buffer_, other.ring_buffer_);
    std::swap(ring_buffer_size_, other.ring_buffer_size_);
    std::swap(ring_buffer_size_exponent_, other.ring_buffer_size_exponent_);
  }
  return *this;
}

PerfEventRingBuffer::~PerfEventRingBuffer() {
  if (metadata_ != nullptr) {
    int munmap_ret = munmap(metadata_, MMAP_LENGTH);
    if (munmap_ret != 0) {
      ERROR("munmap: %s", strerror(errno));
    }
  }
}

bool PerfEventRingBuffer::HasNewData() {
  return metadata_->data_tail + sizeof(perf_event_header) <=
         metadata_->data_head;
}

void PerfEventRingBuffer::ReadHeader(perf_event_header* header) {
  Read(reinterpret_cast<uint8_t*>(header), sizeof(perf_event_header));

  // This must never happen! Reading the buffer failed or the buffer is broken!
  // If this happens, it is probably due to an error in the code,
  //  and we want to abort the excecution.
  assert(header->type != 0);
  assert(metadata_->data_tail + header->size <= metadata_->data_head);
}

void PerfEventRingBuffer::SkipRecord(const perf_event_header& header) {
  // write back how far we read the buffer.
  metadata_->data_tail += header.size;
}

void PerfEventRingBuffer::Read(uint8_t* destination, uint64_t count) {
  const uint64_t index = metadata_->data_tail;
  const uint32_t exponent = ring_buffer_size_exponent_;

  // As ring_buffer_size_ is a power of two, optimize index % ring_buffer_size_
  // to:
  const uint64_t modulo = index & (ring_buffer_size_ - 1);

  // Also we can optimize index / ring_buffer_size_ to:
  const uint64_t index_div_length =
      (index + ((index >> 63) & ((1 << exponent) + ~0llu))) >> exponent;

  const uint64_t index_count = index + count - 1;
  // And also (index + count - 1) / ring_buffer_size_ to:
  const uint64_t index_count_div_length =
      (index_count + ((index_count >> 63) & ((1 << exponent) + ~0llu))) >>
      exponent;

  if (count > ring_buffer_size_) {
    ERROR("Reading more than the size of the ring buffer");
  } else if (metadata_->data_head > metadata_->data_tail + ring_buffer_size_) {
    // If mmap has been called with PROT_WRITE and
    // perf_event_mmap_page::data_tail is used properly, this should not happen,
    // as the kernel would not overwrite unread data.
    ERROR("Too slow reading from the ring buffer");
  } else if (index_div_length == index_count_div_length) {
    memcpy(destination, ring_buffer_ + modulo, count);
  } else if (index_div_length == (index + count - 1) / ring_buffer_size_ - 1) {
    // Need two copies as the data to read wraps around the ring buffer.
    memcpy(destination, ring_buffer_ + modulo, ring_buffer_size_ - modulo);
    memcpy(destination + (ring_buffer_size_ - modulo), ring_buffer_,
           count - (ring_buffer_size_ - modulo));
  } else {
    ERROR("Unexpected error while reading from the ring buffer");
  }
}

void* PerfEventRingBuffer::mmap_mapping(int32_t file_descriptor) {
  // http://man7.org/linux/man-pages/man2/mmap.2.html
  // Use mmap to get access to the ring buffer.
  assert(__builtin_popcount(MMAP_LENGTH - getpagesize()) == 1);
  void* mmap_ret = mmap(nullptr, MMAP_LENGTH, PROT_READ | PROT_WRITE,
                        MAP_SHARED, file_descriptor, 0);
  if (mmap_ret == reinterpret_cast<void*>(-1)) {
    ERROR("mmap: %s", strerror(errno));
  }

  return mmap_ret;
}
