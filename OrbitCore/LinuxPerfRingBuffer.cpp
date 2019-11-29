#include "PrintVar.h"
#include "LinuxPerfRingBuffer.h"
#include "LinuxPerfUtils.h"

#include <assert.h>
#include <linux/perf_event.h>
#include <sys/mman.h>

//-----------------------------------------------------------------------------
LinuxPerfRingBuffer::LinuxPerfRingBuffer(uint32_t a_PerfFileDescriptor)
{
    void* mmap_ret = mmap_mapping(a_PerfFileDescriptor);

    // the memory directly before the ring buffer contains the metadata
    m_Metadata = reinterpret_cast<perf_event_mmap_page*>(mmap_ret);
    m_BufferLength = m_Metadata->data_size;

    // the buffer length must be a power of 2, so find the exponent in order 
    // to later optimize the code using shifts/masks instead of divs/mods.
    uint32_t exponent = 0;
    uint64_t rest = m_BufferLength;
    while( rest >>= 1) ++exponent;
    m_BufferLengthExponent = exponent;

    // beginning of the ring buffer
    m_Buffer = reinterpret_cast<char*>(mmap_ret) + m_Metadata->data_offset; 
}

//-----------------------------------------------------------------------------
LinuxPerfRingBuffer::LinuxPerfRingBuffer(LinuxPerfRingBuffer&& other) noexcept
{
    m_Metadata = other.m_Metadata;
    m_Buffer = other.m_Buffer;
    m_BufferLength = other.m_BufferLength;
    m_BufferLengthExponent = other.m_BufferLengthExponent;
    other.m_Metadata = nullptr;
    other.m_Buffer = nullptr;
    other.m_BufferLength = 0;
    other.m_BufferLengthExponent = 0;
}

//-----------------------------------------------------------------------------
LinuxPerfRingBuffer& LinuxPerfRingBuffer::operator=(LinuxPerfRingBuffer&& other) noexcept
{
    std::swap(m_Metadata, other.m_Metadata);
    std::swap(m_Buffer, other.m_Buffer);
    std::swap(m_BufferLength, other.m_BufferLength);
    std::swap(m_BufferLengthExponent, other.m_BufferLengthExponent);
}

//-----------------------------------------------------------------------------
LinuxPerfRingBuffer::~LinuxPerfRingBuffer()
{
    if (m_Metadata != nullptr)
    {
        // delete the mamory mapping
        int error = munmap(m_Metadata, MMAP_LENGTH);
        if (error == -1)
        {
            PRINT("mmap error: %d\n", errno);
        }
    }
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

//-----------------------------------------------------------------------------
void LinuxPerfRingBuffer::SkipRecord(const perf_event_header& a_Header)
{
    // write back how far we read the buffer.
    m_Metadata->data_tail += a_Header.size;
}

//-----------------------------------------------------------------------------
void LinuxPerfRingBuffer::Read(void* a_Destination, uint64_t a_Count) {
    const uint64_t index = m_Metadata->data_tail;
    const uint32_t exponent = m_BufferLengthExponent;

    // as m_BufferLength is a power of two, we can optimize index % m_BufferLength to:
    const uint64_t modulo = index & (m_BufferLength-1);

    // also we can optimize index / m_BufferLength to:
    const uint64_t index_div_length = 
        (index + ((index >> 63) & ((1 << exponent) + ~0llu))) >> exponent;

    const uint64_t index_count = index + a_Count - 1;
    // and also (index + a_Count - 1) / m_BufferLength
    const uint64_t index_count_div_length = 
        (index_count + ((index_count >> 31) & ((1 << exponent) + ~0))) >> exponent;

    if (a_Count > m_BufferLength)
    {
        // If mmap has been called with PROT_WRITE and
        // perf_event_mmap_page::data_tail is used properly, this should not happen,
        // as the kernel would not overwrite unread data.
        PRINT("Error %u: too slow reading from the ring buffer\n", stderr);
    }
    else if (index_div_length == index_count_div_length)
    {
        memcpy(a_Destination, m_Buffer + modulo, a_Count);
    }
    else if (index_div_length == (index + a_Count - 1) / m_BufferLength - 1)
    {
        // Need two copies as the data to read wraps around the ring buffer.
        memcpy(a_Destination, m_Buffer + modulo,
            m_BufferLength - modulo);
        memcpy(a_Destination + (m_BufferLength - modulo), m_Buffer,
            a_Count - (m_BufferLength - modulo));
    }
    else
    {
        PRINT("Unexpected error while reading from the ring buffer\n");
    }
}

//-----------------------------------------------------------------------------
void* LinuxPerfRingBuffer::mmap_mapping(int32_t a_FileDescriptor)
{
    // http://man7.org/linux/man-pages/man2/mmap.2.html
    // Use mmap to get access to the ring buffer.
    void* mmap_ret =
        mmap(nullptr, MMAP_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, a_FileDescriptor, 0);
    if (mmap_ret == reinterpret_cast<void*>(-1))
    {
        PRINT("mmap error: %d\n", errno);
    }

    return mmap_ret;
}