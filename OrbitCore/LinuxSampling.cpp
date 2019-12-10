#include "Callstack.h"
#include "Capture.h"
#include "CoreApp.h"
#include "LinuxSampling.h"
#include "LinuxPerfUtils.h"
#include "LinuxPerfRingBuffer.h"
#include "LinuxPerfEvent.h"
#include "Path.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "SamplingProfiler.h"
#include "Utils.h"
#include "UnwindStackUtils.h"

#include "unwindstack/Maps.h"
#include "unwindstack/Memory.h"
#include "unwindstack/RegsX86_64.h"
#include "unwindstack/Unwinder.h"
#include "unwindstack/MachineX86_64.h"

#include <fstream>

using namespace LinuxPerfUtils;
using namespace unwindstack;

//-----------------------------------------------------------------------------
LinuxSampling::LinuxSampling( uint32_t a_PID, uint32_t a_Freq )
        : m_PID(a_PID)
        , m_Frequency(a_Freq)
{
}

//-----------------------------------------------------------------------------
void LinuxSampling::Start()
{
    PRINT_FUNC;
    m_ExitRequested = false;

    m_Thread = std::make_shared<std::thread>
        ( &LinuxSampling::RunPerfEventOpen
        , this
        , &m_ExitRequested
        );
    
    m_Thread->detach();
}

//-----------------------------------------------------------------------------
void LinuxSampling::Stop()
{
    PRINT_FUNC;
#if __linux__
    m_ExitRequested = true;
#endif
}

//-----------------------------------------------------------------------------
void LinuxSampling::RunPerfEventOpen( bool* a_ExitRequested )
{
    uint32_t fd = LinuxPerfUtils::sampling_event_open(m_PID, m_Frequency);

    LinuxPerfRingBuffer ring_buffer(fd);

    LinuxPerfUtils::start_capturing(fd);

    std::string maps_buffer = LinuxUtils::RetrieveMaps(m_PID);

    while(!(*a_ExitRequested))
    {
        while(ring_buffer.HasNewData())
        {
            perf_event_header header{};
            ring_buffer.ReadHeader(&header);

            switch (header.type)
            {
                case PERF_RECORD_SAMPLE:
                    {
                        auto record = ring_buffer.ConsumeRecord<LinuxSamplingEvent>(header);
                        const perf_sample_regs_user& regs_content = record.Regs();

                        // TODO: Do this in a seperate thread
                        RegsX86_64 regs = UnwindStackUtils::LoadRegisters(regs_content);

                        BufferMaps maps{maps_buffer.c_str()};
                        if (!maps.Parse()) {
                            PRINT("FAILED READING MMAP\n");
                            continue;
                        }

                        std::shared_ptr<Memory> memory =
                            Memory::CreateOfflineMemory(
                                reinterpret_cast<const uint8_t*>(record.StackDump()), 
                                regs_content.SP, 
                                regs_content.SP + record.StackSize()
                            );
                        Unwinder unwinder{128, &maps, &regs, memory};
                        unwinder.Unwind();

                        if (unwinder.LastErrorCode() != 0) {
                            PRINT("Unwinding failed. Error code: %d\n", unwinder.LastErrorCode());
                            continue;
                        }

                        CallStack CS;

                        for (uint64_t i = 0; i < unwinder.NumFrames(); ++i) {
                            UnwindStackUtils::ProcessStackFrame(i, unwinder, CS);
                        }

                        CS.m_Depth = (uint32_t)CS.m_Data.size();

                        Capture::GSamplingProfiler->AddCallStack( CS );
                        GEventTracer.GetEventBuffer().AddCallstackEvent( record.Timestamp(), CS.Hash(), record.TID() );
                    }
                    break;

                case PERF_RECORD_MMAP:
                    // For that process id, there was a call to mmap with PROT_EXEC
                    // (i.e. loaded a new library). So we need to refresh the maps.
                    // This should however happen very rarely, so we can do this inplace.
                    maps_buffer = LinuxUtils::RetrieveMaps(m_PID);
                    break;
                case PERF_RECORD_LOST:
                    {
                        auto lost = ring_buffer.ConsumeRecord<LinuxPerfLostEvent>(header);
                        PRINT("Lost %u Events\n", lost.Lost());
                    }
                    break;
                default:
                    PRINT("Unexpected Perf Sample Type: %u", header.type);
                    ring_buffer.SkipRecord(header);
                    break;
            }

        }
        OrbitSleepMs(1);
    }

    LinuxPerfUtils::stop_capturing(fd);
}