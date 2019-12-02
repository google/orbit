//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------
#pragma once

#include "LinuxPerfUtils.h"
#include <linux/perf_event.h>

class LinuxPerfRingBuffer
{
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

    //-----------------------------------------------------------------------------
    template<typename LinuxPerfEvent>
    LinuxPerfEvent ConsumeRecord(const perf_event_header& a_Header)
    {
        LinuxPerfEvent record;

        // perf_event_header::size contains the size of the entire record.
        // As record is an object of type LinuxPerfEvent, it has the vtable
        // at offset 0. The data fields start at the address of the header.
        Read(&record.header, a_Header.size);

        SkipRecord(a_Header);

        return record;
    }

private:
    const size_t RING_BUFFER_PAGE_COUNT = 2048; // 64: 256 KB; 2048: 8 MB
    const size_t PAGE_SIZE = getpagesize();  // 4 KB
    // "The mmap size should be 1+2^n pages, where the first page is a meta‐
    // data page (struct perf_event_mmap_page) that contains various bits of
    // information such as where the ring-buffer head is."
    // http://man7.org/linux/man-pages/man2/perf_event_open.2.html
    const size_t MMAP_LENGTH = (1 + RING_BUFFER_PAGE_COUNT) * PAGE_SIZE;

    perf_event_mmap_page* m_Metadata;
    char* m_Buffer;
    uint64_t m_BufferLength;
    // the buffer length must be a power of 2, so we can do shifting for division.
    uint32_t m_BufferLengthExponent;
    
    void Read(void* a_Destination, uint64_t a_Count);

    void* mmap_mapping(int32_t a_FileDescriptor);
};