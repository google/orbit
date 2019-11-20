//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------
#pragma once

#include "LinuxPerfUtils.h"
#include <linux/perf_event.h>

class LinuxPerfRingBuffer
{
public:
    LinuxPerfRingBuffer(uint32_t a_PerfFileDescriptor);
    bool HasNewData();
    void ReadHeader(perf_event_header* a_Header);
    void SkipRecord(const perf_event_header& a_Header);

    //-----------------------------------------------------------------------------
    template<typename T>
    T ConsumeRecord(const perf_event_header& a_Header)
    {
        T record;

        // perf_event_header::size contains the size of the entire record.
        Read(&record, a_Header.size);

        SkipRecord(a_Header);

        return record;
    }

private:
    perf_event_mmap_page* m_Metadata;
    char* m_Buffer;
    uint64_t m_BufferLength;
    // the buffer length must be a power of 2, so we can do shifting for division.
    uint32_t m_BufferLengthExponent;

    //-----------------------------------------------------------------------------
    void Read(void* a_Destination, uint64_t a_Count);
};