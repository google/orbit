// Copyright (C) 2020-2022 Intel Corporation
// SPDX-License-Identifier: MIT

#include <assert.h>
#include <stddef.h>
#include <windows.h>
#include <evntcons.h> // must include after windows.h

#include "TraceSession.hpp"

#include "Debug.hpp"
#include "PresentMonTraceConsumer.hpp"
#include "MixedRealityTraceConsumer.hpp"

#include "ETW/Microsoft_Windows_D3D9.h"
#include "ETW/Microsoft_Windows_Dwm_Core.h"
#include "ETW/Microsoft_Windows_DXGI.h"
#include "ETW/Microsoft_Windows_DxgKrnl.h"
#include "ETW/Microsoft_Windows_EventMetadata.h"
#include "ETW/Microsoft_Windows_Win32k.h"
#include "ETW/NT_Process.h"

namespace {

struct TraceProperties : public EVENT_TRACE_PROPERTIES {
    wchar_t mSessionName[MAX_PATH];
};

struct FilteredProvider {
    EVENT_FILTER_DESCRIPTOR filterDesc_;
    ENABLE_TRACE_PARAMETERS params_;
    uint64_t anyKeywordMask_;
    uint64_t allKeywordMask_;
    uint8_t maxLevel_;

    FilteredProvider(
        GUID const& sessionGuid,
        bool filterEventIds)
    {
        memset(&filterDesc_, 0, sizeof(filterDesc_));
        memset(&params_,     0, sizeof(params_));

        anyKeywordMask_ = 0;
        allKeywordMask_ = 0;
        maxLevel_ = 0;

        if (filterEventIds) {
            static_assert(MAX_EVENT_FILTER_EVENT_ID_COUNT >= ANYSIZE_ARRAY, "Unexpected MAX_EVENT_FILTER_EVENT_ID_COUNT");
            auto memorySize = sizeof(EVENT_FILTER_EVENT_ID) + sizeof(USHORT) * (MAX_EVENT_FILTER_EVENT_ID_COUNT - ANYSIZE_ARRAY);
            void* memory = _aligned_malloc(memorySize, alignof(USHORT));
            if (memory != nullptr) {
                auto filteredEventIds = (EVENT_FILTER_EVENT_ID*) memory;
                filteredEventIds->FilterIn = TRUE;
                filteredEventIds->Reserved = 0;
                filteredEventIds->Count = 0;

                filterDesc_.Ptr = (ULONGLONG) filteredEventIds;
                filterDesc_.Size = (ULONG) memorySize;
                filterDesc_.Type = EVENT_FILTER_TYPE_EVENT_ID;

                params_.Version = ENABLE_TRACE_PARAMETERS_VERSION_2;
                params_.EnableProperty = EVENT_ENABLE_PROPERTY_IGNORE_KEYWORD_0;
                params_.SourceId = sessionGuid;
                params_.EnableFilterDesc = &filterDesc_;
                params_.FilterDescCount = 1;
            }
        }
    }

    ~FilteredProvider()
    {
        if (filterDesc_.Ptr != 0) {
            auto memory = (void*) filterDesc_.Ptr;
            _aligned_free(memory);
        }
    }

    void ClearFilter()
    {
        if (filterDesc_.Ptr != 0) {
            auto filteredEventIds = (EVENT_FILTER_EVENT_ID*) filterDesc_.Ptr;
            filteredEventIds->Count = 0;
        }

        anyKeywordMask_ = 0;
        allKeywordMask_ = 0;
        maxLevel_ = 0;
    }

    void AddKeyword(uint64_t keyword)
    {
        if (anyKeywordMask_ == 0) {
            anyKeywordMask_ = keyword;
            allKeywordMask_ = keyword;
        } else {
            anyKeywordMask_ |= keyword;
            allKeywordMask_ &= keyword;
        }
    }

    template<typename T>
    void AddEvent()
    {
        if (filterDesc_.Ptr != 0) {
            auto filteredEventIds = (EVENT_FILTER_EVENT_ID*) filterDesc_.Ptr;
            assert(filteredEventIds->Count < MAX_EVENT_FILTER_EVENT_ID_COUNT);
            filteredEventIds->Events[filteredEventIds->Count++] = T::Id;
        }

        #pragma warning(suppress: 4984) // C++17 extension
        if constexpr ((uint64_t) T::Keyword != 0ull) {
            AddKeyword((uint64_t) T::Keyword);
        }

        maxLevel_ = max(maxLevel_, T::Level);
    }

    ULONG Enable(
        TRACEHANDLE sessionHandle,
        GUID const& providerGuid)
    {
        ENABLE_TRACE_PARAMETERS* pparams = nullptr;
        if (filterDesc_.Ptr != 0) {
            pparams = &params_;

            // EnableTraceEx2() failes unless Size agrees with Count.
            auto filterEventIds = (EVENT_FILTER_EVENT_ID*) filterDesc_.Ptr;
            filterDesc_.Size = sizeof(EVENT_FILTER_EVENT_ID) + sizeof(USHORT) * (filterEventIds->Count - ANYSIZE_ARRAY);
        }

        ULONG timeout = 0;
        return EnableTraceEx2(sessionHandle, &providerGuid, EVENT_CONTROL_CODE_ENABLE_PROVIDER,
                              maxLevel_, anyKeywordMask_, allKeywordMask_, timeout, pparams);
    }
};

ULONG EnableProviders(
    TRACEHANDLE sessionHandle,
    GUID const& sessionGuid,
    PMTraceConsumer* pmConsumer,
    MRTraceConsumer* mrConsumer)
{
    ULONG status = 0;

    // Lookup what OS we're running on
    //
    // We can't use helpers like IsWindows8Point1OrGreater() since they FALSE
    // if the application isn't built with a manifest.
    bool isWin81OrGreater = false;
    bool isWin11OrGreater = false;
    {
        auto hmodule = LoadLibraryExA("ntdll.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (hmodule != NULL) {
            auto pRtlGetVersion = (LONG (WINAPI*)(RTL_OSVERSIONINFOW*)) GetProcAddress(hmodule, "RtlGetVersion");
            if (pRtlGetVersion != nullptr) {
                RTL_OSVERSIONINFOW info = {};
                info.dwOSVersionInfoSize = sizeof(info);
                status = (*pRtlGetVersion)(&info);
                if (status == 0 /* STATUS_SUCCESS */) {
                    // win8.1 = version 6.3
                    // win11  = version 10.0 build >= 22000
                    isWin81OrGreater = info.dwMajorVersion >  6 || (info.dwMajorVersion ==  6 && info.dwMinorVersion >= 3);
                    isWin11OrGreater = info.dwMajorVersion > 10 || (info.dwMajorVersion == 10 && info.dwBuildNumber  >= 22000);
                }
            }
            FreeLibrary(hmodule);
        }
    }

    // Scope filtering based on event ID only works on Win8.1 or greater.
    bool filterEventIds = isWin81OrGreater;
    pmConsumer->mFilteredEvents = filterEventIds;

    // Start backend providers first to reduce Presents being queued up before
    // we can track them.
    FilteredProvider provider(sessionGuid, filterEventIds);

    // Microsoft_Windows_DxgKrnl
    provider.ClearFilter();
    provider.AddEvent<Microsoft_Windows_DxgKrnl::PresentHistory_Start>();
    if (pmConsumer->mTrackDisplay) {
        provider.AddEvent<Microsoft_Windows_DxgKrnl::Blit_Info>();
        provider.AddEvent<Microsoft_Windows_DxgKrnl::BlitCancel_Info>();
        provider.AddEvent<Microsoft_Windows_DxgKrnl::Flip_Info>();
        provider.AddEvent<Microsoft_Windows_DxgKrnl::IndependentFlip_Info>();
        provider.AddEvent<Microsoft_Windows_DxgKrnl::FlipMultiPlaneOverlay_Info>();
        provider.AddEvent<Microsoft_Windows_DxgKrnl::HSyncDPCMultiPlane_Info>();
        provider.AddEvent<Microsoft_Windows_DxgKrnl::VSyncDPCMultiPlane_Info>();
        provider.AddEvent<Microsoft_Windows_DxgKrnl::MMIOFlip_Info>();
        provider.AddEvent<Microsoft_Windows_DxgKrnl::MMIOFlipMultiPlaneOverlay_Info>();
        provider.AddEvent<Microsoft_Windows_DxgKrnl::Present_Info>();
        provider.AddEvent<Microsoft_Windows_DxgKrnl::PresentHistory_Info>();
        provider.AddEvent<Microsoft_Windows_DxgKrnl::PresentHistoryDetailed_Start>();
        provider.AddEvent<Microsoft_Windows_DxgKrnl::QueuePacket_Start>();
        provider.AddEvent<Microsoft_Windows_DxgKrnl::QueuePacket_Stop>();
        provider.AddEvent<Microsoft_Windows_DxgKrnl::VSyncDPC_Info>();
    }
    // BEGIN WORKAROUND: Windows11 adds a "Present" keyword to:
    //     BlitCancel_Info
    //     Blit_Info
    //     FlipMultiPlaneOverlay_Info
    //     Flip_Info
    //     HSyncDPCMultiPlane_Info
    //     MMIOFlipMultiPlaneOverlay_Info
    //     MMIOFlip_Info
    //     PresentHistoryDetailed_Start
    //     PresentHistory_Info
    //     PresentHistory_Start
    //     Present_Info
    //     VSyncDPC_Info
    if (isWin11OrGreater) {
        provider.AddKeyword((uint64_t) Microsoft_Windows_DxgKrnl::Keyword::Microsoft_Windows_DxgKrnl_Performance |
                            (uint64_t) Microsoft_Windows_DxgKrnl::Keyword::Base |
                            (uint64_t) Microsoft_Windows_DxgKrnl::Keyword::Present);
    }
    // END WORKAROUND
    // BEGIN WORKAROUND: Don't filter DXGK events using the Performance keyword,
    // as that can have side-effects with negative performance impact on some
    // versions of Windows.
    provider.anyKeywordMask_ &= ~(uint64_t) Microsoft_Windows_DxgKrnl::Keyword::Microsoft_Windows_DxgKrnl_Performance;
    provider.allKeywordMask_ &= ~(uint64_t) Microsoft_Windows_DxgKrnl::Keyword::Microsoft_Windows_DxgKrnl_Performance;
    // END WORKAROUND
    status = provider.Enable(sessionHandle, Microsoft_Windows_DxgKrnl::GUID);
    if (status != ERROR_SUCCESS) return status;

    status = EnableTraceEx2(sessionHandle, &Microsoft_Windows_DxgKrnl::Win7::GUID, EVENT_CONTROL_CODE_ENABLE_PROVIDER,
                            TRACE_LEVEL_INFORMATION, 0, 0, 0, nullptr);
    if (status != ERROR_SUCCESS) return status;

    if (pmConsumer->mTrackDisplay) {
        // Microsoft_Windows_Win32k
        provider.ClearFilter();
        provider.AddEvent<Microsoft_Windows_Win32k::TokenCompositionSurfaceObject_Info>();
        provider.AddEvent<Microsoft_Windows_Win32k::TokenStateChanged_Info>();
        status = provider.Enable(sessionHandle, Microsoft_Windows_Win32k::GUID);
        if (status != ERROR_SUCCESS) return status;

        // Microsoft_Windows_Dwm_Core
        provider.ClearFilter();
        provider.AddEvent<Microsoft_Windows_Dwm_Core::MILEVENT_MEDIA_UCE_PROCESSPRESENTHISTORY_GetPresentHistory_Info>();
        provider.AddEvent<Microsoft_Windows_Dwm_Core::SCHEDULE_PRESENT_Start>();
        provider.AddEvent<Microsoft_Windows_Dwm_Core::SCHEDULE_SURFACEUPDATE_Info>();
        provider.AddEvent<Microsoft_Windows_Dwm_Core::FlipChain_Pending>();
        provider.AddEvent<Microsoft_Windows_Dwm_Core::FlipChain_Complete>();
        provider.AddEvent<Microsoft_Windows_Dwm_Core::FlipChain_Dirty>();
        // BEGIN WORKAROUND: Windows11 uses Scheduling keyword instead of DwmCore keyword for:
        //     SCHEDULE_PRESENT_Start
        //     SCHEDULE_SURFACEUPDATE_Info
        if (isWin11OrGreater) {
            provider.AddKeyword((uint64_t) Microsoft_Windows_Dwm_Core::Keyword::Microsoft_Windows_Dwm_Core_Diagnostic |
                                (uint64_t) Microsoft_Windows_Dwm_Core::Keyword::Scheduling);
        }
        // END WORKAROUND
        status = provider.Enable(sessionHandle, Microsoft_Windows_Dwm_Core::GUID);
        if (status != ERROR_SUCCESS) return status;

        status = EnableTraceEx2(sessionHandle, &Microsoft_Windows_Dwm_Core::Win7::GUID, EVENT_CONTROL_CODE_ENABLE_PROVIDER,
                                TRACE_LEVEL_VERBOSE, 0, 0, 0, nullptr);
        if (status != ERROR_SUCCESS) return status;
    }

    // Microsoft_Windows_DXGI
    provider.ClearFilter();
    provider.AddEvent<Microsoft_Windows_DXGI::Present_Start>();
    provider.AddEvent<Microsoft_Windows_DXGI::Present_Stop>();
    provider.AddEvent<Microsoft_Windows_DXGI::PresentMultiplaneOverlay_Start>();
    provider.AddEvent<Microsoft_Windows_DXGI::PresentMultiplaneOverlay_Stop>();
    status = provider.Enable(sessionHandle, Microsoft_Windows_DXGI::GUID);
    if (status != ERROR_SUCCESS) return status;

    // Microsoft_Windows_D3D9
    provider.ClearFilter();
    provider.AddEvent<Microsoft_Windows_D3D9::Present_Start>();
    provider.AddEvent<Microsoft_Windows_D3D9::Present_Stop>();
    status = provider.Enable(sessionHandle, Microsoft_Windows_D3D9::GUID);
    if (status != ERROR_SUCCESS) return status;

    if (mrConsumer != nullptr) {
        // DHD
        status = EnableTraceEx2(sessionHandle, &DHD_PROVIDER_GUID, EVENT_CONTROL_CODE_ENABLE_PROVIDER,
                                TRACE_LEVEL_VERBOSE, 0x1C00000, 0, 0, nullptr);
        if (status != ERROR_SUCCESS) return status;

        if (!mrConsumer->mSimpleMode) {
            // SPECTRUMCONTINUOUS
            status = EnableTraceEx2(sessionHandle, &SPECTRUMCONTINUOUS_PROVIDER_GUID, EVENT_CONTROL_CODE_ENABLE_PROVIDER,
                                    TRACE_LEVEL_VERBOSE, 0x800000, 0, 0, nullptr);
            if (status != ERROR_SUCCESS) return status;
        }
    }

    return ERROR_SUCCESS;
}

void DisableProviders(TRACEHANDLE sessionHandle)
{
    ULONG status = 0;
    status = EnableTraceEx2(sessionHandle, &Microsoft_Windows_DXGI::GUID,           EVENT_CONTROL_CODE_DISABLE_PROVIDER, 0, 0, 0, 0, nullptr);
    status = EnableTraceEx2(sessionHandle, &Microsoft_Windows_D3D9::GUID,           EVENT_CONTROL_CODE_DISABLE_PROVIDER, 0, 0, 0, 0, nullptr);
    status = EnableTraceEx2(sessionHandle, &Microsoft_Windows_DxgKrnl::GUID,        EVENT_CONTROL_CODE_DISABLE_PROVIDER, 0, 0, 0, 0, nullptr);
    status = EnableTraceEx2(sessionHandle, &Microsoft_Windows_Win32k::GUID,         EVENT_CONTROL_CODE_DISABLE_PROVIDER, 0, 0, 0, 0, nullptr);
    status = EnableTraceEx2(sessionHandle, &Microsoft_Windows_Dwm_Core::GUID,       EVENT_CONTROL_CODE_DISABLE_PROVIDER, 0, 0, 0, 0, nullptr);
    status = EnableTraceEx2(sessionHandle, &Microsoft_Windows_Dwm_Core::Win7::GUID, EVENT_CONTROL_CODE_DISABLE_PROVIDER, 0, 0, 0, 0, nullptr);
    status = EnableTraceEx2(sessionHandle, &Microsoft_Windows_DxgKrnl::Win7::GUID,  EVENT_CONTROL_CODE_DISABLE_PROVIDER, 0, 0, 0, 0, nullptr);
    status = EnableTraceEx2(sessionHandle, &DHD_PROVIDER_GUID,                      EVENT_CONTROL_CODE_DISABLE_PROVIDER, 0, 0, 0, 0, nullptr);
    status = EnableTraceEx2(sessionHandle, &SPECTRUMCONTINUOUS_PROVIDER_GUID,       EVENT_CONTROL_CODE_DISABLE_PROVIDER, 0, 0, 0, 0, nullptr);
}

template<
    bool SAVE_FIRST_TIMESTAMP,
    bool TRACK_DISPLAY,
    bool TRACK_WMR>
void CALLBACK EventRecordCallback(EVENT_RECORD* pEventRecord)
{
    auto session = (TraceSession*) pEventRecord->UserContext;
    auto const& hdr = pEventRecord->EventHeader;

    #pragma warning(push)
    #pragma warning(disable: 4984) // c++17 extension

    if constexpr (SAVE_FIRST_TIMESTAMP) {
        if (session->mStartQpc.QuadPart == 0) {
            session->mStartQpc = hdr.TimeStamp;
        }
    }

    if (hdr.ProviderId == Microsoft_Windows_DxgKrnl::GUID) {
        session->mPMConsumer->HandleDXGKEvent(pEventRecord);
        return;
    }
    if (hdr.ProviderId == Microsoft_Windows_DXGI::GUID) {
        session->mPMConsumer->HandleDXGIEvent(pEventRecord);
        return;
    }
    if (hdr.ProviderId == Microsoft_Windows_D3D9::GUID) {
        session->mPMConsumer->HandleD3D9Event(pEventRecord);
        return;
    }
    if (hdr.ProviderId == NT_Process::GUID) {
        session->mPMConsumer->HandleNTProcessEvent(pEventRecord);
        return;
    }
    if (hdr.ProviderId == Microsoft_Windows_DxgKrnl::Win7::PRESENTHISTORY_GUID) {
        session->mPMConsumer->HandleWin7DxgkPresentHistory(pEventRecord);
        return;
    }
    if (hdr.ProviderId == Microsoft_Windows_EventMetadata::GUID) {
        session->mPMConsumer->HandleMetadataEvent(pEventRecord);
        return;
    }

    if constexpr (TRACK_DISPLAY) {
        if (hdr.ProviderId == Microsoft_Windows_Win32k::GUID) {
            session->mPMConsumer->HandleWin32kEvent(pEventRecord);
            return;
        }
    }

    if constexpr (TRACK_DISPLAY) {
        if (hdr.ProviderId == Microsoft_Windows_Dwm_Core::GUID) {
            session->mPMConsumer->HandleDWMEvent(pEventRecord);
            return;
        }
        if (hdr.ProviderId == Microsoft_Windows_Dwm_Core::Win7::GUID) {
            session->mPMConsumer->HandleDWMEvent(pEventRecord);
            return;
        }
        if (hdr.ProviderId == Microsoft_Windows_DxgKrnl::Win7::BLT_GUID) {
            session->mPMConsumer->HandleWin7DxgkBlt(pEventRecord);
            return;
        }
        if (hdr.ProviderId == Microsoft_Windows_DxgKrnl::Win7::FLIP_GUID) {
            session->mPMConsumer->HandleWin7DxgkFlip(pEventRecord);
            return;
        }
        if (hdr.ProviderId == Microsoft_Windows_DxgKrnl::Win7::QUEUEPACKET_GUID) {
            session->mPMConsumer->HandleWin7DxgkQueuePacket(pEventRecord);
            return;
        }
        if (hdr.ProviderId == Microsoft_Windows_DxgKrnl::Win7::VSYNCDPC_GUID) {
            session->mPMConsumer->HandleWin7DxgkVSyncDPC(pEventRecord);
            return;
        }
        if (hdr.ProviderId == Microsoft_Windows_DxgKrnl::Win7::MMIOFLIP_GUID) {
            session->mPMConsumer->HandleWin7DxgkMMIOFlip(pEventRecord);
            return;
        }

        if constexpr (TRACK_WMR) {
            if (hdr.ProviderId == SPECTRUMCONTINUOUS_PROVIDER_GUID) {
                session->mMRConsumer->HandleSpectrumContinuousEvent(pEventRecord);
                return;
            }
        }
    }

    if constexpr (TRACK_WMR) {
        if (hdr.ProviderId == DHD_PROVIDER_GUID) {
            session->mMRConsumer->HandleDHDEvent(pEventRecord);
            return;
        }
    }

    #pragma warning(pop)
}

template<bool SAVE_FIRST_TIMESTAMP, bool TRACK_DISPLAY>
PEVENT_RECORD_CALLBACK GetEventRecordCallback(bool trackWMR)
{
    return trackWMR ? &EventRecordCallback<SAVE_FIRST_TIMESTAMP, TRACK_DISPLAY, true>
                    : &EventRecordCallback<SAVE_FIRST_TIMESTAMP, TRACK_DISPLAY, false>;
}

template<bool SAVE_FIRST_TIMESTAMP, bool TRACK_DISPLAY>
PEVENT_RECORD_CALLBACK GetEventRecordCallback(bool trackInput, bool trackWMR, bool trackINTC, bool trackPower)
{
    return trackInput ? GetEventRecordCallback<SAVE_FIRST_TIMESTAMP, TRACK_DISPLAY, true>(trackWMR, trackINTC, trackPower)
                      : GetEventRecordCallback<SAVE_FIRST_TIMESTAMP, TRACK_DISPLAY, false>(trackWMR, trackINTC, trackPower);
}

template<bool SAVE_FIRST_TIMESTAMP>
PEVENT_RECORD_CALLBACK GetEventRecordCallback(bool trackDisplay, bool trackWMR)
{
    return trackDisplay ? GetEventRecordCallback<SAVE_FIRST_TIMESTAMP, true>(trackWMR)
                        : GetEventRecordCallback<SAVE_FIRST_TIMESTAMP, false>(trackWMR);
}

PEVENT_RECORD_CALLBACK GetEventRecordCallback(bool saveFirstTimestamp, bool trackDisplay, bool trackWMR)
{
    return saveFirstTimestamp ? GetEventRecordCallback<true>(trackDisplay, trackWMR)
                              : GetEventRecordCallback<false>(trackDisplay, trackWMR);
}

ULONG CALLBACK BufferCallback(EVENT_TRACE_LOGFILEA* pLogFile)
{
    auto session = (TraceSession*) pLogFile->Context;
    return session->mContinueProcessingBuffers; // TRUE = continue processing events, FALSE = return out of ProcessTrace()
}

}

ULONG TraceSession::Start(
    PMTraceConsumer* pmConsumer,
    MRTraceConsumer* mrConsumer,
    char const* etlPath,
    char const* sessionName)
{
    assert(mSessionHandle == 0);
    assert(mTraceHandle == INVALID_PROCESSTRACE_HANDLE);
    mStartQpc.QuadPart = 0;
    mPMConsumer = pmConsumer;
    mMRConsumer = mrConsumer;
    mContinueProcessingBuffers = TRUE;

    // -------------------------------------------------------------------------
    // Configure trace properties
    EVENT_TRACE_LOGFILEA traceProps = {};
    traceProps.LogFileName = (char*) etlPath;
    traceProps.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD | PROCESS_TRACE_MODE_RAW_TIMESTAMP;
    traceProps.Context = this;
    /* Output members (passed also to BufferCallback):
    traceProps.CurrentTime
    traceProps.BuffersRead
    traceProps.CurrentEvent
    traceProps.LogfileHeader
    traceProps.BufferSize
    traceProps.Filled
    traceProps.IsKernelTrace
    */

    // Redirect to a specialized event handler based on the tracking parameters.
    auto saveFirstTimestamp = etlPath != nullptr;
    traceProps.EventRecordCallback = GetEventRecordCallback(
        saveFirstTimestamp,
        pmConsumer->mTrackDisplay,
        mrConsumer != nullptr);

    // When processing log files, we need to use the buffer callback in case
    // the user wants to stop processing before the entire log has been parsed.
    if (traceProps.LogFileName != nullptr) {
        traceProps.BufferCallback = &BufferCallback;
    }

    // -------------------------------------------------------------------------
    // For realtime collection, start the session with the required providers
    if (traceProps.LogFileName == nullptr) {
        traceProps.LoggerName = (char*) sessionName;
        traceProps.ProcessTraceMode |= PROCESS_TRACE_MODE_REAL_TIME;

        TraceProperties sessionProps = {};
        sessionProps.Wnode.BufferSize = (ULONG) sizeof(TraceProperties);
        sessionProps.Wnode.ClientContext = 1;                     // Clock resolution to use when logging the timestamp for each event; 1 == query performance counter
        sessionProps.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;    // We have a realtime consumer, not writing to a log file
        sessionProps.LogFileNameOffset = 0;                       // 0 means no output log file
        sessionProps.LoggerNameOffset = offsetof(TraceProperties, mSessionName);  // Location of session name; will be written by StartTrace()
        /* Not used:
        sessionProps.Wnode.Guid               // Only needed for private or kernel sessions, otherwise it's an output
        sessionProps.FlushTimer               // How often in seconds buffers are flushed; 0=min (1 second)
        sessionProps.EnableFlags              // Which kernel providers to include in trace
        sessionProps.AgeLimit                 // n/a
        sessionProps.BufferSize = 0;          // Size of each tracing buffer in kB (max 1MB)
        sessionProps.MinimumBuffers = 200;    // Min tracing buffer pool size; must be at least 2 per processor
        sessionProps.MaximumBuffers = 0;      // Max tracing buffer pool size; min+20 by default
        sessionProps.MaximumFileSize = 0;     // Max file size in MB
        */
        /* The following members are output variables, set by StartTrace() and/or ControlTrace()
        sessionProps.Wnode.HistoricalContext  // handle to the event tracing session
        sessionProps.Wnode.TimeStamp          // time this structure was updated
        sessionProps.Wnode.Guid               // session Guid
        sessionProps.Wnode.Flags              // e.g., WNODE_FLAG_TRACED_GUID
        sessionProps.NumberOfBuffers          // trace buffer pool size
        sessionProps.FreeBuffers              // trace buffer pool free count
        sessionProps.EventsLost               // count of events not written
        sessionProps.BuffersWritten           // buffers written in total
        sessionProps.LogBuffersLost           // buffers that couldn't be written to the log
        sessionProps.RealTimeBuffersLost      // buffers that couldn't be delivered to the realtime consumer
        sessionProps.LoggerThreadId           // tracing session identifier
        */

        auto status = StartTraceA(&mSessionHandle, sessionName, &sessionProps);
        if (status != ERROR_SUCCESS) {
            mSessionHandle = 0;
            return status;
        }

        status = EnableProviders(mSessionHandle, sessionProps.Wnode.Guid, pmConsumer, mrConsumer);
        if (status != ERROR_SUCCESS) {
            Stop();
            return status;
        }
    }

    // -------------------------------------------------------------------------
    // Open the trace
    mTraceHandle = OpenTraceA(&traceProps);
    if (mTraceHandle == INVALID_PROCESSTRACE_HANDLE) {
        auto lastError = GetLastError();
        Stop();
        return lastError;
    }

    // -------------------------------------------------------------------------
    // Save the initial time to base capture off of.  ETL captures use the
    // time of the first event, which matches GPUVIEW usage, and realtime
    // captures are based off the timestamp here.

    switch (traceProps.LogfileHeader.ReservedFlags) {
    case 2: // System time
        mQpcFrequency.QuadPart = 10000000ull;
        break;
    case 3: // CPU cycle counter
        mQpcFrequency.QuadPart = 1000000ull * traceProps.LogfileHeader.CpuSpeedInMHz;
        break;
    default: // 1 == QPC
        mQpcFrequency = traceProps.LogfileHeader.PerfFreq;
        break;
    }

    if (!saveFirstTimestamp) {
        QueryPerformanceCounter(&mStartQpc);
    }

    DebugInitialize(&mStartQpc, mQpcFrequency);

    return ERROR_SUCCESS;
}

void TraceSession::Stop()
{
    ULONG status = 0;

    // If collecting realtime events, CloseTrace() will cause ProcessTrace() to
    // stop filling buffers and it will return after it finishes processing
    // events already in it's buffers.
    //
    // If collecting from a log file, ProcessTrace() will continue to process
    // the entire file though, which is why we cancel the processing from the
    // BufferCallback in this case.
    mContinueProcessingBuffers = FALSE;

    // Shutdown the trace and session.
    status = CloseTrace(mTraceHandle);
    mTraceHandle = INVALID_PROCESSTRACE_HANDLE;

    if (mSessionHandle != 0) {
        DisableProviders(mSessionHandle);

        TraceProperties sessionProps = {};
        sessionProps.Wnode.BufferSize = (ULONG) sizeof(TraceProperties);
        sessionProps.LoggerNameOffset = offsetof(TraceProperties, mSessionName);
        status = ControlTraceW(mSessionHandle, nullptr, &sessionProps, EVENT_TRACE_CONTROL_STOP);

        mSessionHandle = 0;
    }
}

ULONG TraceSession::StopNamedSession(char const* sessionName)
{
    TraceProperties sessionProps = {};
    sessionProps.Wnode.BufferSize = (ULONG) sizeof(TraceProperties);
    sessionProps.LoggerNameOffset = offsetof(TraceProperties, mSessionName);
    return ControlTraceA((TRACEHANDLE) 0, sessionName, &sessionProps, EVENT_TRACE_CONTROL_STOP);
}


ULONG TraceSession::CheckLostReports(ULONG* eventsLost, ULONG* buffersLost) const
{
    TraceProperties sessionProps = {};
    sessionProps.Wnode.BufferSize = (ULONG) sizeof(TraceProperties);
    sessionProps.LoggerNameOffset = offsetof(TraceProperties, mSessionName);

    auto status = ControlTraceW(mSessionHandle, nullptr, &sessionProps, EVENT_TRACE_CONTROL_QUERY);
    *eventsLost = sessionProps.EventsLost;
    *buffersLost = sessionProps.RealTimeBuffersLost;
    return status;
}
