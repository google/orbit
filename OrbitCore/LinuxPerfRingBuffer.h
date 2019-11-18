//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------
#pragma once

#if __linux__

#include "LinuxPerfUtils.h"
#include <linux/perf_event.h>

class LinuxPerfRingBuffer
{
public:
    LinuxPerfRingBuffer(uint32_t perf_file_descriptor);
    bool HasNewData();
    void ReadHeader(perf_event_header* header);
    void SkipRecord(const perf_event_header& header);

    //-----------------------------------------------------------------------------
    template<typename T>
    T ConsumeRecord(const perf_event_header& header)
    {
        T record;

        // perf_event_header::size contains the size of the entire record.
        Read(&record, header.size);

        SkipRecord(header);

        return record;
    }

private:
    perf_event_mmap_page* m_Metadata;
    char* m_Buffer;
    uint64_t m_BufferLength;

    //-----------------------------------------------------------------------------
    void Read(void* dest, uint64_t count);
};

#endif