// Copyright (C) 2017-2020 Intel Corporation
// SPDX-License-Identifier: MIT

#define NOMINMAX
#include <algorithm>
#include <d3d9.h>
#include <dxgi.h>
#include <windows.h>
#include <tdh.h>

#include "MixedRealityTraceConsumer.hpp"
#include "TraceConsumer.hpp"
#include "ETW/Microsoft_Windows_DxgKrnl.h"

namespace {

std::wstring GetEventTaskNameFromTdh(EVENT_RECORD* pEventRecord)
{
    std::wstring taskName = L"";
    ULONG bufferSize = 0;
    auto status = TdhGetEventInformation(pEventRecord, 0, nullptr, nullptr, &bufferSize);
    if (status == ERROR_INSUFFICIENT_BUFFER) {
        auto bufferAddr = malloc(bufferSize);
        if (bufferAddr != nullptr) {

            auto info = (TRACE_EVENT_INFO*)bufferAddr;
            status = TdhGetEventInformation(pEventRecord, 0, nullptr, info, &bufferSize);
            if (status == ERROR_SUCCESS) {
                taskName = (wchar_t*)((uintptr_t) bufferAddr + info->TaskNameOffset);
            }

            free(bufferAddr);
        }
    }

    return taskName;
}

}

HolographicFrame::HolographicFrame(EVENT_HEADER const& hdr)
    : PresentId(0)
    , FrameId(0)
    , StartTime(*(uint64_t*)&hdr.TimeStamp)
    , StopTime(0)
    , ProcessId(hdr.ProcessId)
    , Completed(false)
    , FinalState(HolographicFrameResult::Unknown)
{
}

PresentationSource::PresentationSource()
    : PresentationSource(0)
{
}

PresentationSource::PresentationSource(uint64_t ptr)
    : Ptr(ptr)
    , AcquireForRenderingTime(0)
    , ReleaseFromRenderingTime(0)
    , AcquireForPresentationTime(0)
    , ReleaseFromPresentationTime(0)
    , pHolographicFrame(nullptr)
{
}

PresentationSource::~PresentationSource()
{
}

LateStageReprojectionEvent::LateStageReprojectionEvent(EVENT_HEADER const& hdr)
    : QpcTime(*(uint64_t*) &hdr.TimeStamp)
    , NewSourceLatched(false)
    , ThreadWakeupStartLatchToCpuRenderFrameStartInMs(0)
    , CpuRenderFrameStartToHeadPoseCallbackStartInMs(0)
    , HeadPoseCallbackStartToHeadPoseCallbackStopInMs(0)
    , HeadPoseCallbackStopToInputLatchInMs(0)
    , InputLatchToGpuSubmissionInMs(0)
    , GpuSubmissionToGpuStartInMs(0)
    , GpuStartToGpuStopInMs(0)
    , GpuStopToCopyStartInMs(0)
    , CopyStartToCopyStopInMs(0)
    , CopyStopToVsyncInMs(0)
    , LsrPredictionLatencyMs(0)
    , AppPredictionLatencyMs(0)
    , AppMispredictionMs(0)
    , TotalWakeupErrorMs(0)
    , TimeUntilVsyncMs(0)
    , TimeUntilPhotonsMiddleMs(0)
    , ProcessId(hdr.ProcessId)
    , FinalState(LateStageReprojectionResult::Unknown)
    , MissedVsyncCount(0)
    , Completed(false)
{
}

void MRTraceConsumer::CompleteLSR(std::shared_ptr<LateStageReprojectionEvent> p)
{
    if (p->FinalState == LateStageReprojectionResult::Unknown) {
        return;
    }

    if (p->Completed) {
        p->FinalState = LateStageReprojectionResult::Error;
        return;
    }

    p->Completed = true;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mCompletedLSRs.push_back(p);
    }
}

void MRTraceConsumer::CompleteHolographicFrame(std::shared_ptr<HolographicFrame> p)
{
    if (p->Completed) {
        p->FinalState = HolographicFrameResult::Error;
        return;
    }

    // Remove it from any tracking structures that it may have been inserted into.
    mHolographicFramesByPresentId.erase(p->PresentId);

    p->Completed = true;
}

void MRTraceConsumer::CompletePresentationSource(uint64_t presentationSourcePtr)
{
    // Remove it from any tracking structures that it may have been inserted into.
    mPresentationSourceByPtr.erase(presentationSourcePtr);
}

decltype(MRTraceConsumer::mPresentationSourceByPtr.begin()) MRTraceConsumer::FindOrCreatePresentationSource(uint64_t presentationSourcePtr)
{
    // See if we already have a presentation source.
    auto sourceIter = mPresentationSourceByPtr.find(presentationSourcePtr);
    if (sourceIter != mPresentationSourceByPtr.end()) {
        return sourceIter;
    }

    // Create a new presentation source.
    auto newSource = std::make_shared<PresentationSource>(presentationSourcePtr);
    sourceIter = mPresentationSourceByPtr.emplace(presentationSourcePtr, newSource).first;
    return sourceIter;
}

void MRTraceConsumer::HolographicFrameStart(std::shared_ptr<HolographicFrame> p)
{
    const auto frameIter = mHolographicFramesByFrameId.find(p->FrameId);
    const bool bHolographicFrameIdExists = (frameIter != mHolographicFramesByFrameId.end());
    if (bHolographicFrameIdExists) {
        // Collision with an existing in-flight Holographic FrameId. This should be rare/transient.
        // Timing information for the source may be wrong if it get's timing from the wrong Holographic Frame.
        frameIter->second->FinalState = HolographicFrameResult::DuplicateFrameId;
        p->FinalState = HolographicFrameResult::DuplicateFrameId;

        // Set the event instance to completed so the assert
        // in ~HolographicFrame() doesn't fire when it is destructed.
        frameIter->second->Completed = true;
    }

    mHolographicFramesByFrameId[p->FrameId] = p;
}

void MRTraceConsumer::HolographicFrameStop(std::shared_ptr<HolographicFrame> p)
{
    // Remove the frame from being tracked by FrameId.
    // Begin tracking the frame by its PresentId until LSR picks it up.
    mHolographicFramesByFrameId.erase(p->FrameId);

    assert(p->PresentId != 0 && p->StopTime != 0);
    if (p->FinalState == HolographicFrameResult::Unknown) {
        p->FinalState = HolographicFrameResult::Presented;
    }
    mHolographicFramesByPresentId.emplace(p->PresentId, p);
}

void MRTraceConsumer::HandleDHDEvent(EVENT_RECORD* pEventRecord)
{
    auto const& hdr = pEventRecord->EventHeader;
    const std::wstring taskName = GetEventTaskNameFromTdh(pEventRecord);

    if (taskName.compare(L"AcquireForRendering") == 0)
    {
        const uint64_t ptr = mMetadata.GetEventData<uint64_t>(pEventRecord, L"thisPtr");
        auto sourceIter = FindOrCreatePresentationSource(ptr);
        sourceIter->second->AcquireForRenderingTime = *(uint64_t*)&hdr.TimeStamp;

        // Clear old timing data in case the Presentation Source is reused.
        sourceIter->second->ReleaseFromRenderingTime = 0;
        sourceIter->second->AcquireForPresentationTime = 0;
        sourceIter->second->ReleaseFromPresentationTime = 0;
    }
    else if (taskName.compare(L"ReleaseFromRendering") == 0)
    {
        const uint64_t ptr = mMetadata.GetEventData<uint64_t>(pEventRecord, L"thisPtr");
        auto sourceIter = FindOrCreatePresentationSource(ptr);
        sourceIter->second->ReleaseFromRenderingTime = *(uint64_t*)&hdr.TimeStamp;
    }
    else if (taskName.compare(L"AcquireForPresentation") == 0)
    {
        const uint64_t ptr = mMetadata.GetEventData<uint64_t>(pEventRecord, L"thisPtr");
        auto sourceIter = FindOrCreatePresentationSource(ptr);
        sourceIter->second->AcquireForPresentationTime = *(uint64_t*)&hdr.TimeStamp;
    }
    else if (taskName.compare(L"ReleaseFromPresentation") == 0)
    {
        const uint64_t ptr = mMetadata.GetEventData<uint64_t>(pEventRecord, L"thisPtr");
        auto sourceIter = FindOrCreatePresentationSource(ptr);
        sourceIter->second->ReleaseFromPresentationTime = *(uint64_t*)&hdr.TimeStamp;

        // Update the active LSR event based on the latest info in the source.
        // Note: We take a snapshot (copy) the data.
        auto& pEvent = mActiveLSR;
        if (pEvent) {
            pEvent->Source = *sourceIter->second;
        }
    }
    else if (taskName.compare(L"OasisPresentationSource") == 0)
    {
        std::string eventType = mMetadata.GetEventData<std::string>(pEventRecord, L"EventType");
        eventType.pop_back(); // Pop the null-terminator so the compare works.
        if (eventType.compare("Destruction") == 0) {
            const uint64_t ptr = mMetadata.GetEventData<uint64_t>(pEventRecord, L"thisPtr");
            CompletePresentationSource(ptr);
        }
    }
    else if (taskName.compare(L"LsrThread_BeginLsrProcessing") == 0)
    {
        // Complete the last LSR.
        auto& pEvent = mActiveLSR;
        if (pEvent) {
            CompleteLSR(pEvent);
        }

        // Start a new LSR.
        pEvent = std::make_shared<LateStageReprojectionEvent>(hdr);

        EventDataDesc desc[] = {
            { L"SourcePtr" },
            { L"NewSourceLatched" },
            { L"TimeUntilVblankMs" },
            { L"TimeUntilPhotonsMiddleMs" },
            { L"PredictionSampleTimeToPhotonsVisibleMs" },
            { L"MispredictionMs" },
        };
        mMetadata.GetEventData(pEventRecord, desc, _countof(desc));
        pEvent->Source.Ptr =               desc[0].GetData<uint64_t>();
        pEvent->NewSourceLatched =         desc[1].GetData<bool    >();
        pEvent->TimeUntilVsyncMs =         desc[2].GetData<float   >();
        pEvent->TimeUntilPhotonsMiddleMs = desc[3].GetData<float   >();
        pEvent->AppPredictionLatencyMs =   desc[4].GetData<float   >();
        pEvent->AppMispredictionMs =       desc[5].GetData<float   >();

        assert(pEvent->Source.Ptr != 0);
    }
    else if (taskName.compare(L"LsrThread_LatchedInput") == 0)
    {
        // Update the active LSR.
        auto& pEvent = mActiveLSR;
        if (pEvent) {
            // New pose latched.
            EventDataDesc desc[] = {
                { L"TimeUntilTopPhotonsMs" },
                { L"TimeUntilBottomPhotonsMs" },
            };
            mMetadata.GetEventData(pEventRecord, desc, _countof(desc));
            const float timeUntilPhotonsTopMs    = desc[0].GetData<float>();
            const float timeUntilPhotonsBottomMs = desc[1].GetData<float>();
            const float timeUntilPhotonsMiddleMs = (timeUntilPhotonsTopMs + timeUntilPhotonsBottomMs) / 2;
            pEvent->LsrPredictionLatencyMs = timeUntilPhotonsMiddleMs;

            if (!mSimpleMode) {
                // Get the latest details about the Holographic Frame being used for presentation.
                // Link Presentation Source -> Holographic Frame using the PresentId.
                const uint32_t presentId = mMetadata.GetEventData<uint32_t>(pEventRecord, L"PresentId");
                auto frameIter = mHolographicFramesByPresentId.find(presentId);
                if (frameIter != mHolographicFramesByPresentId.end()) {
                    // Now that we've latched, the source has been acquired for presentation.
                    auto sourceIter = FindOrCreatePresentationSource(pEvent->Source.Ptr);
                    assert(sourceIter->second->AcquireForPresentationTime != 0);

                    // Update the source with information about the Holographic Frame being used.
                    sourceIter->second->pHolographicFrame = frameIter->second;

                    // Done with this Holographic Frame.
                    CompleteHolographicFrame(frameIter->second);
                }
            }
         }
    }
    else if (taskName.compare(L"LsrThread_UnaccountedForVsyncsBetweenStatGathering") == 0)
    {
        // Update the active LSR.
        auto& pEvent = mActiveLSR;
        if (pEvent) {
            // We have missed some extra Vsyncs we need to account for.
            const uint32_t unaccountedForMissedVSyncCount = mMetadata.GetEventData<uint32_t>(pEventRecord, L"unaccountedForVsyncsBetweenStatGathering");
            assert(unaccountedForMissedVSyncCount >= 1);
            pEvent->MissedVsyncCount += unaccountedForMissedVSyncCount;
        }
    }
    else if (taskName.compare(L"MissedPresentation") == 0)
    {
        // Update the active LSR.
        auto& pEvent = mActiveLSR;
        if (pEvent) {
            // If the missed reason is for Present, increment our missed Vsync count.
            const uint32_t MissedReason = mMetadata.GetEventData<uint32_t>(pEventRecord, L"reason");
            if (MissedReason == 0) {
                pEvent->MissedVsyncCount++;
            }
        }
    }
    else if (taskName.compare(L"OnTimePresentationTiming") == 0 || taskName.compare(L"LatePresentationTiming") == 0)
    {
        // Update the active LSR.
        auto& pEvent = mActiveLSR;
        if (pEvent) {
            EventDataDesc desc[] = {
                { L"cpuRenderFrameStartToHeadPoseCallbackStartInMs" },
                { L"headPoseCallbackDurationInMs" },
                { L"headPoseCallbackEndToInputLatchInMs" },
                { L"inputLatchToGpuSubmissionInMs" },
                { L"gpuSubmissionToGpuStartInMs" },
                { L"gpuStartToGpuStopInMs" },
                { L"gpuStopToCopyStartInMs" },
                { L"copyStartToCopyStopInMs" },
                { L"copyStopToVsyncInMs" },
                { L"frameSubmittedOnSchedule" },
                // Newer versions of the event have changed property names,
                // only one of the following is expected to be found:
                { L"startLatchToCpuRenderFrameStartInMs" }, { L"threadWakeupToCpuRenderFrameStartInMs" },
                { L"totalWakeupErrorMs" },                  { L"wakeupErrorInMs" },
            };
            mMetadata.GetEventData(pEventRecord, desc, _countof(desc));
            pEvent->CpuRenderFrameStartToHeadPoseCallbackStartInMs =  desc[0].GetData<float>();
            pEvent->HeadPoseCallbackStartToHeadPoseCallbackStopInMs = desc[1].GetData<float>();
            pEvent->HeadPoseCallbackStopToInputLatchInMs =            desc[2].GetData<float>();
            pEvent->InputLatchToGpuSubmissionInMs =                   desc[3].GetData<float>();
            pEvent->GpuSubmissionToGpuStartInMs =                     desc[4].GetData<float>();
            pEvent->GpuStartToGpuStopInMs =                           desc[5].GetData<float>();
            pEvent->GpuStopToCopyStartInMs =                          desc[6].GetData<float>();
            pEvent->CopyStartToCopyStopInMs =                         desc[7].GetData<float>();
            pEvent->CopyStopToVsyncInMs =                             desc[8].GetData<float>();
            auto bFrameSubmittedOnSchedule =                          desc[9].GetData<bool>();

            // Check which name was found and use that data...
            pEvent->ThreadWakeupStartLatchToCpuRenderFrameStartInMs = desc[10].data_ == nullptr
                ? desc[11].GetData<float>()
                : desc[10].GetData<float>();
            pEvent->TotalWakeupErrorMs = desc[12].data_ == nullptr
                ? desc[13].GetData<float>()
                : desc[12].GetData<float>();

            if (bFrameSubmittedOnSchedule) {
                pEvent->FinalState = LateStageReprojectionResult::Presented;
            }
            else {
                pEvent->FinalState = (pEvent->MissedVsyncCount > 1) ? LateStageReprojectionResult::MissedMultiple : LateStageReprojectionResult::Missed;
            }
        }
    }
}

void MRTraceConsumer::HandleSpectrumContinuousEvent(EVENT_RECORD* pEventRecord)
{
    auto const& hdr = pEventRecord->EventHeader;
    const std::wstring taskName = GetEventTaskNameFromTdh(pEventRecord);

    if (taskName.compare(L"HolographicFrame") == 0)
    {
        // Ignore rehydrated frames.
        const bool bIsRehydration = mMetadata.GetEventData<bool>(pEventRecord, L"isRehydration");
        if (!bIsRehydration) {
            switch (pEventRecord->EventHeader.EventDescriptor.Opcode)
            {
            case EVENT_TRACE_TYPE_START:
            {
                // CreateNextFrame() was called by the App.
                auto pFrame = std::make_shared<HolographicFrame>(hdr);
                pFrame->FrameId = mMetadata.GetEventData<uint32_t>(pEventRecord, L"holographicFrameID");

                HolographicFrameStart(pFrame);
                break;
            }
            case EVENT_TRACE_TYPE_STOP:
            {
                // PresentUsingCurrentPrediction() was called by the App.
                const uint32_t holographicFrameId = mMetadata.GetEventData<uint32_t>(pEventRecord, L"holographicFrameID");
                auto frameIter = mHolographicFramesByFrameId.find(holographicFrameId);
                if (frameIter == mHolographicFramesByFrameId.end()) {
                    return;
                }

                const uint64_t timeStamp = *(uint64_t*)&hdr.TimeStamp;
                assert(frameIter->second->StartTime <= timeStamp);
                frameIter->second->StopTime = timeStamp;

                // Only stop the frame once we've seen all the events for it.
                if (frameIter->second->PresentId != 0 && frameIter->second->StopTime != 0) {
                    HolographicFrameStop(frameIter->second);
                }
                break;
            }
            }
        }
    }
    else if (taskName.compare(L"HolographicFrameMetadata_GetNewPoseForReprojection") == 0)
    {
        // Link holographicFrameId -> presentId.
        const uint32_t holographicFrameId = mMetadata.GetEventData<uint32_t>(pEventRecord, L"holographicFrameId");
        auto frameIter = mHolographicFramesByFrameId.find(holographicFrameId);
        if (frameIter == mHolographicFramesByFrameId.end()) {
            return;
        }

        frameIter->second->PresentId = mMetadata.GetEventData<uint32_t>(pEventRecord, L"presentId");

        // Only complete the frame once we've seen all the events for it.
        if (frameIter->second->PresentId != 0 && frameIter->second->StopTime != 0) {
            HolographicFrameStop(frameIter->second);
        }
    }
}
