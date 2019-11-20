#include "PrintVar.h"
#include "LinuxPerfRingBuffer.h"
#include "LinuxPerfUtils.h"

#include <assert.h>
#include <linux/perf_event.h>

//-----------------------------------------------------------------------------
LinuxPerfRingBuffer::LinuxPerfRingBuffer(uint32_t a_PerfFileDescriptor)
{
    void* mmap_ret = LinuxPerfUtils::mmap_mapping(a_PerfFileDescriptor);

    // the memory directly before the ring buffer contains the metadata
    m_Metadata = reinterpret_cast<perf_event_mmap_page*>(mmap_ret);
    m_BufferLength = m_Metadata->data_size;

    // beginning of the ring buffer
    m_Buffer = reinterpret_cast<char*>(mmap_ret) + m_Metadata->data_offset; 
}

//-----------------------------------------------------------------------------
bool LinuxPerfRingBuffer::HasNewData() 
{
    return m_Metadata->data_tail + sizeof(perf_event_header) <= m_Metadata->data_head;
}

//-----------------------------------------------------------------------------
void LinuxPerfRingBuffer::ReadHeader(perf_event_header* a_Header)
{
    Read(reinterpret_cast<char*>(a_Header), sizeof(perf_event_header));

    // This must never happen! Reading the buffer failed or the buffer is broken!
    // If this happens, it is probably due to an error in the code, 
    //  and we want to abort the excecution.
    assert(a_Header->type != 0);
    assert(m_Metadata->data_tail + a_Header->size <= m_Metadata->data_head);
}


void LinuxPerfRingBuffer::SkipRecord(const perf_event_header& a_Header)
{
    // write back how far we read the buffer.
    m_Metadata->data_tail += a_Header.size;
}

//-----------------------------------------------------------------------------
void LinuxPerfRingBuffer::Read(void* a_Destination, uint64_t a_Count) {
    uint64_t index = m_Metadata->data_tail;
    if (a_Count > m_BufferLength) {
        // If mmap has been called with PROT_WRITE and
        // perf_event_mmap_page::data_tail is used properly, this should not happen,
        // as the kernel would no overwrite unread data.
        PRINT("Error %u: too slow reading from the ring buffer\n", stderr);
    } else if (index / m_BufferLength == (index + a_Count - 1) / m_BufferLength) {
        memcpy(a_Destination, m_Buffer + index % m_BufferLength, a_Count);
    } else if (index / m_BufferLength == (index + a_Count - 1) / m_BufferLength - 1) {
        // Need two copies as the data to read wraps around the ring buffer.
        memcpy(a_Destination, m_Buffer + index % m_BufferLength,
            m_BufferLength - index % m_BufferLength);
        memcpy(a_Destination + (m_BufferLength - index % m_BufferLength), m_Buffer,
            a_Count - (m_BufferLength - index % m_BufferLength));
    } else {
        PRINT("Unexpected error while reading from the ring buffer\n");
    }
}