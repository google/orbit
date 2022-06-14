// Copyright (C) 2017-2020 Intel Corporation
// SPDX-License-Identifier: MIT

#pragma once

#include <assert.h>
#include <deque>
#include <map>
#include <mutex>
#include <numeric>
#include <set>
#include <vector>
#include <windows.h>
#include <evntcons.h> // must include after windows.h

#include "PresentMonTraceConsumer.hpp"

struct __declspec(uuid("{356e1338-04ad-420e-8b8a-a2eb678541cf}")) SPECTRUMCONTINUOUS_PROVIDER_GUID_HOLDER;
struct __declspec(uuid("{19d9d739-da0a-41a0-b97f-24ed27abc9fb}")) DHD_PROVIDER_GUID_HOLDER;
static const auto SPECTRUMCONTINUOUS_PROVIDER_GUID = __uuidof(SPECTRUMCONTINUOUS_PROVIDER_GUID_HOLDER);
static const auto DHD_PROVIDER_GUID = __uuidof(DHD_PROVIDER_GUID_HOLDER);

enum class HolographicFrameResult
{
    Unknown, Presented, DuplicateFrameId, Error
};

// A HolographicFrame is created by the Windows Mixed Reality App or Shell.
// A HolographicFrame's lifetime is short (span of a couple frames), just long enough
// to capture data about how the App or Shell used that frame. Data comes from the Spectrum Continuous provider.
struct HolographicFrame {
    uint32_t PresentId; // PresentId: Unique globally
    uint32_t FrameId;   // HolographicFrameId: Unique per-process

    uint64_t StartTime; // Qpc when CreateNextFrame() is called.
    uint64_t StopTime;  // Qpc when PresentWithCurrentPrediction() is called.

    uint32_t ProcessId;
    bool Completed;
    HolographicFrameResult FinalState;

    HolographicFrame(EVENT_HEADER const& hdr);

    inline uint64_t GetCpuRenderFrameTime() const
    {
        if (StartTime > 0 && StopTime > 0)
        {
            assert(StopTime >= StartTime);
            return StopTime - StartTime;
        }
        else
        {
            return 0;
        }
    }

    inline uint64_t GetPresentTime() const
    {
        return StopTime;
    }
};

struct PresentationSource {
    uint64_t Ptr;
    uint64_t AcquireForRenderingTime;   // Qpc time the Presentation Source was acquired for rendering by DWM.
    uint64_t ReleaseFromRenderingTime;  // Qpc time the Presentation Source was released from rendering by DWM and Gpu work is submitted (Note LSR will only pick it up if the Gpu work is complete).
    uint64_t AcquireForPresentationTime;    // Qpc time the Presentation Source was acquired for LSR (the Gpu work is required to be complete).
    uint64_t ReleaseFromPresentationTime;   // Qpc time the Presentation Source was released from LSR.

    std::shared_ptr<HolographicFrame> pHolographicFrame;

    PresentationSource();
    PresentationSource(uint64_t ptr);
    ~PresentationSource();

    inline uint64_t GetReleaseFromRenderingToAcquireForPresentationTime() const
    {
        if (ReleaseFromRenderingTime > 0 && AcquireForPresentationTime > 0)
        {
            assert(AcquireForPresentationTime >= ReleaseFromRenderingTime);
            return AcquireForPresentationTime - ReleaseFromRenderingTime;
        }
        else
        {
            return 0;
        }
    }
};

enum class LateStageReprojectionResult
{
    Unknown, Presented, Missed, MissedMultiple, Error
};

inline bool LateStageReprojectionPresented(LateStageReprojectionResult result)
{
    return (result == LateStageReprojectionResult::Presented) ? true : false;
}

inline bool LateStageReprojectionMissed(LateStageReprojectionResult result)
{
    switch (result)
    {
    case LateStageReprojectionResult::Missed:
    case LateStageReprojectionResult::MissedMultiple:
        return true;
    }

    return false;
}

// A LateStageReprojectionEvent is used to track a single instance of LSR.
// A LateStageReprojectionEvent's lifetime is short (span of a couple frames), just long enough to capture data about that LSR.
// Data comes from the DHD provider.
struct LateStageReprojectionEvent {
    uint64_t QpcTime;

    PresentationSource Source;  // A copy of the PresentationSource used when the input was latched.
    bool NewSourceLatched;

    float ThreadWakeupStartLatchToCpuRenderFrameStartInMs;
    float CpuRenderFrameStartToHeadPoseCallbackStartInMs;
    float HeadPoseCallbackStartToHeadPoseCallbackStopInMs;
    float HeadPoseCallbackStopToInputLatchInMs;
    float InputLatchToGpuSubmissionInMs;
    float GpuSubmissionToGpuStartInMs;
    float GpuStartToGpuStopInMs;
    float GpuStopToCopyStartInMs;
    float CopyStartToCopyStopInMs;
    float CopyStopToVsyncInMs;

    float LsrPredictionLatencyMs;
    float AppPredictionLatencyMs;
    float AppMispredictionMs;
    float TotalWakeupErrorMs;
    float TimeUntilVsyncMs;
    float TimeUntilPhotonsMiddleMs;

    uint32_t ProcessId;
    LateStageReprojectionResult FinalState;
    uint32_t MissedVsyncCount;

    // Additional transient state
    bool Completed;

    LateStageReprojectionEvent(EVENT_HEADER const& hdr);

    inline bool IsValidAppFrame() const
    {
        return Source.pHolographicFrame != nullptr;
    }

    inline uint32_t GetAppFrameId() const
    {
        return Source.pHolographicFrame ? Source.pHolographicFrame->FrameId : 0;
    }

    inline uint32_t GetAppProcessId() const
    {
        return Source.pHolographicFrame ? Source.pHolographicFrame->ProcessId : 0;
    }

    inline uint64_t GetAppPresentTime() const
    {
        return Source.pHolographicFrame ? Source.pHolographicFrame->GetPresentTime() : 0;
    }

    inline uint64_t GetAppCpuRenderFrameTime() const
    {
        return Source.pHolographicFrame ? Source.pHolographicFrame->GetCpuRenderFrameTime() : 0;
    }

    inline float GetLsrCpuRenderFrameMs() const
    {
        return CpuRenderFrameStartToHeadPoseCallbackStartInMs +
            HeadPoseCallbackStartToHeadPoseCallbackStopInMs +
            HeadPoseCallbackStopToInputLatchInMs +
            InputLatchToGpuSubmissionInMs;
    }

    inline float GetLsrThreadWakeupStartLatchToGpuEndMs() const
    {
        return ThreadWakeupStartLatchToCpuRenderFrameStartInMs +
            CpuRenderFrameStartToHeadPoseCallbackStartInMs +
            HeadPoseCallbackStartToHeadPoseCallbackStopInMs +
            HeadPoseCallbackStopToInputLatchInMs +
            InputLatchToGpuSubmissionInMs +
            GpuSubmissionToGpuStartInMs +
            GpuStartToGpuStopInMs +
            GpuStopToCopyStartInMs +
            CopyStartToCopyStopInMs;
    }
    
    inline float GetLsrMotionToPhotonLatencyMs() const
    {
        return InputLatchToGpuSubmissionInMs +
            GpuSubmissionToGpuStartInMs +
            GpuStartToGpuStopInMs +
            GpuStopToCopyStartInMs +
            CopyStartToCopyStopInMs +
            CopyStopToVsyncInMs +
            (TimeUntilPhotonsMiddleMs - TimeUntilVsyncMs);
    }
};

struct MRTraceConsumer
{
    MRTraceConsumer(bool simple)
        : mSimpleMode(simple)
    {}

    EventMetadata mMetadata;

    const bool mSimpleMode;

    std::mutex mMutex;
    // A set of LSRs that are "completed":
    // They progressed as far as they can through the pipeline before being either discarded or hitting the screen.
    // These will be handed off to the consumer thread.
    std::vector<std::shared_ptr<LateStageReprojectionEvent>> mCompletedLSRs;

    // A high-level description of the sequence of events:
    // HolographicFrameStart (by HolographicFrameId, for App's CPU frame render start time) -> HolographicFrameStop (by HolographicFrameId, for App's CPU frame render end/Present time) -> 
    //  AcquireForRendering (by PresentationSource, for DWM's CPU frame compose start time) -> ReleaseFromRendering (by PresentationSource, for DWM's CPU frame compose end/GPU Submit time) ->
    //  BeginLsrProcessing (by PresentId and PresentationSource, for LSR's start time) -> AcquireForPresentation (by PresentationSource, for LSR's CPU frame render start time) -> HolographicFrameMetadata_GetNewPoseForReprojection (by HolographicFrameId and PresentId, for linking HolographicFrameId to PresentId) ->
    //  LatchedInput (by PresentId, for LSR's pose latency) -> ReleaseFromPresentation (by PresentationSource, for LSR's CPU frame end/GPU Submit time) -> OnTimePresentationTiming/LatePresentationTiming (for detailed LSR timing information)

    // Presentation Sources being used by the app.
    std::map<uint64_t, std::shared_ptr<PresentationSource>> mPresentationSourceByPtr;

    // Stores each Holographic Frame started by it's HolographicFrameId.
    std::map<uint32_t, std::shared_ptr<HolographicFrame>> mHolographicFramesByFrameId;

    // Stores each Holographic Frame started by it's PresentId.
    std::map<uint32_t, std::shared_ptr<HolographicFrame>> mHolographicFramesByPresentId;

    std::shared_ptr<LateStageReprojectionEvent> mActiveLSR;
    void DequeueLSRs(std::vector<std::shared_ptr<LateStageReprojectionEvent>>& outLSRs)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        outLSRs.swap(mCompletedLSRs);
    }

    void CompleteLSR(std::shared_ptr<LateStageReprojectionEvent> p);
    void CompleteHolographicFrame(std::shared_ptr<HolographicFrame> p);
    void CompletePresentationSource(uint64_t presentationSourcePtr);

    decltype(mPresentationSourceByPtr.begin()) FindOrCreatePresentationSource(uint64_t presentationSourcePtr);
    
    void HolographicFrameStart(std::shared_ptr<HolographicFrame> p);
    void HolographicFrameStop(std::shared_ptr<HolographicFrame> p);

    void HandleDHDEvent(EVENT_RECORD* pEventRecord);
    void HandleSpectrumContinuousEvent(EVENT_RECORD* pEventRecord);
};

