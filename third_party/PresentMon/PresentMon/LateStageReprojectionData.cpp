// Copyright (C) 2019-2022 Intel Corporation
// SPDX-License-Identifier: MIT

#include "PresentMon.hpp"

#include <algorithm>

enum {
    MAX_HISTORY_TIME = 3000,
    LSR_TIMEOUT_THRESHOLD_TICKS = 10000, // 10 sec
    MAX_LSRS_IN_DEQUE = 120 * (MAX_HISTORY_TIME / 1000)
};

void LateStageReprojectionData::PruneDeque(std::deque<LateStageReprojectionEvent> &lsrHistory, uint32_t msTimeDiff, uint32_t maxHistLen) {
    while (!lsrHistory.empty() && (
        lsrHistory.size() > maxHistLen ||
        1000.0 * QpcDeltaToSeconds(lsrHistory.back().QpcTime - lsrHistory.front().QpcTime) > msTimeDiff)) {
        lsrHistory.pop_front();
    }
}

void LateStageReprojectionData::AddLateStageReprojection(LateStageReprojectionEvent& p)
{
    if (LateStageReprojectionPresented(p.FinalState))
    {
        assert(p.MissedVsyncCount == 0);
        mDisplayedLSRHistory.push_back(p);
    }
    else if(LateStageReprojectionMissed(p.FinalState))
    {
        assert(p.MissedVsyncCount >= 1);
        mLifetimeLsrMissedFrames += p.MissedVsyncCount;
    }

    if (p.NewSourceLatched)
    {
        mSourceHistory.push_back(p);
    }
    else
    {
        mLifetimeAppMissedFrames++;
    }

    if (!mLSRHistory.empty())
    {
        assert(mLSRHistory.back().QpcTime <= p.QpcTime);
    }
    mLSRHistory.push_back(p);
}

void LateStageReprojectionData::UpdateLateStageReprojectionInfo()
{
    PruneDeque(mSourceHistory, MAX_HISTORY_TIME, MAX_LSRS_IN_DEQUE);
    PruneDeque(mDisplayedLSRHistory, MAX_HISTORY_TIME, MAX_LSRS_IN_DEQUE);
    PruneDeque(mLSRHistory, MAX_HISTORY_TIME, MAX_LSRS_IN_DEQUE);
}

double LateStageReprojectionData::ComputeHistoryTime(const std::deque<LateStageReprojectionEvent>& lsrHistory) const
{
    if (lsrHistory.size() < 2) {
        return 0.0;
    }

    auto start = lsrHistory.front().QpcTime;
    auto end = lsrHistory.back().QpcTime;
    return QpcDeltaToSeconds(end - start);
}

size_t LateStageReprojectionData::ComputeHistorySize() const
{
    if (mLSRHistory.size() < 2) {
        return 0;
    }

    return mLSRHistory.size();
}

double LateStageReprojectionData::ComputeHistoryTime() const
{
    return ComputeHistoryTime(mLSRHistory);
}

double LateStageReprojectionData::ComputeFps(const std::deque<LateStageReprojectionEvent>& lsrHistory) const
{
    if (lsrHistory.size() < 2) {
        return 0.0;
    }
    auto start = lsrHistory.front().QpcTime;
    auto end = lsrHistory.back().QpcTime;
    auto count = lsrHistory.size() - 1;

    return count / QpcDeltaToSeconds(end - start);
}

double LateStageReprojectionData::ComputeSourceFps() const
{
    return ComputeFps(mSourceHistory);
}

double LateStageReprojectionData::ComputeDisplayedFps() const
{
    return ComputeFps(mDisplayedLSRHistory);
}

double LateStageReprojectionData::ComputeFps() const
{
    return ComputeFps(mLSRHistory);
}

LateStageReprojectionRuntimeStats LateStageReprojectionData::ComputeRuntimeStats() const
{
    LateStageReprojectionRuntimeStats stats = {};
    if (mLSRHistory.size() < 2) {
        return stats;
    }

    uint64_t totalAppSourceReleaseToLsrAcquireTime = 0;
    uint64_t totalAppSourceCpuRenderTime = 0;
    const size_t count = mLSRHistory.size();
    for (size_t i = 0; i < count; i++)
    {
        LateStageReprojectionEvent const& current = mLSRHistory[i];

        stats.mGpuPreemptionInMs.AddValue(current.GpuSubmissionToGpuStartInMs);
        stats.mGpuExecutionInMs.AddValue(current.GpuStartToGpuStopInMs);
        stats.mCopyPreemptionInMs.AddValue(current.GpuStopToCopyStartInMs);
        stats.mCopyExecutionInMs.AddValue(current.CopyStartToCopyStopInMs);

        const double lsrInputLatchToVsyncInMs = (double)
            current.InputLatchToGpuSubmissionInMs +
            current.GpuSubmissionToGpuStartInMs +
            current.GpuStartToGpuStopInMs +
            current.GpuStopToCopyStartInMs +
            current.CopyStartToCopyStopInMs +
            current.CopyStopToVsyncInMs;
        stats.mLsrInputLatchToVsyncInMs.AddValue(lsrInputLatchToVsyncInMs);

        // Stats just with averages
        totalAppSourceReleaseToLsrAcquireTime += current.Source.GetReleaseFromRenderingToAcquireForPresentationTime();
        totalAppSourceCpuRenderTime += current.GetAppCpuRenderFrameTime();
        stats.mLsrCpuRenderTimeInMs += (double)
            current.CpuRenderFrameStartToHeadPoseCallbackStartInMs +
            current.HeadPoseCallbackStartToHeadPoseCallbackStopInMs +
            current.HeadPoseCallbackStopToInputLatchInMs +
            current.InputLatchToGpuSubmissionInMs;

        stats.mGpuEndToVsyncInMs += current.CopyStopToVsyncInMs;
        stats.mVsyncToPhotonsMiddleInMs += (double) current.TimeUntilPhotonsMiddleMs - current.TimeUntilVsyncMs;
        stats.mLsrPoseLatencyInMs += current.LsrPredictionLatencyMs;
        stats.mAppPoseLatencyInMs += current.AppPredictionLatencyMs;

        if (!current.NewSourceLatched) {
            stats.mAppMissedFrames++;
        }

        if (LateStageReprojectionMissed(current.FinalState)) {
            stats.mLsrMissedFrames += current.MissedVsyncCount;
            if (current.MissedVsyncCount > 1) {
                // We always expect a count of at least 1, but if we missed multiple vsyncs during a single LSR period we need to account for that.
                stats.mLsrConsecutiveMissedFrames += (current.MissedVsyncCount - 1);
            }
            if (i > 0 && LateStageReprojectionMissed((mLSRHistory[i - 1].FinalState))) {
                stats.mLsrConsecutiveMissedFrames++;
            }
        }
    }

    stats.mAppProcessId = mLSRHistory[count - 1].GetAppProcessId();
    stats.mLsrProcessId = mLSRHistory[count - 1].ProcessId;

    stats.mAppSourceCpuRenderTimeInMs = 1000.0 * QpcDeltaToSeconds(totalAppSourceCpuRenderTime);
    stats.mAppSourceReleaseToLsrAcquireInMs = 1000.0 * QpcDeltaToSeconds(totalAppSourceReleaseToLsrAcquireTime);

    stats.mAppSourceReleaseToLsrAcquireInMs /= count;
    stats.mAppSourceCpuRenderTimeInMs /= count;
    stats.mLsrCpuRenderTimeInMs /= count;
    stats.mGpuEndToVsyncInMs /= count;
    stats.mVsyncToPhotonsMiddleInMs /= count;
    stats.mLsrPoseLatencyInMs /= count;
    stats.mAppPoseLatencyInMs /= count;

    return stats;
}

FILE* CreateLsrCsvFile(char const* path)
{
    auto const& args = GetCommandLineArgs();

    // Add _WMR to the file name
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char name[_MAX_FNAME];
    char ext[_MAX_EXT];
    _splitpath_s(path, drive, dir, name, ext);

    char outputPath[MAX_PATH] = {};
    _snprintf_s(outputPath, _TRUNCATE, "%s%s%s_WMR%s", drive, dir, name, ext);

    // Open output file
    FILE* fp = nullptr;
    if (fopen_s(&fp, outputPath, "w")) {
        return nullptr;
    }

    // Print CSV header
    fprintf(fp, "Application,ProcessID,DwmProcessID");
    if (args.mTrackDebug) {
        fprintf(fp, ",HolographicFrameID");
    }
    fprintf(fp, ",TimeInSeconds");
    if (args.mTrackDisplay) {
        fprintf(fp, ",msBetweenAppPresents,msAppPresentToLsr");
    }
    fprintf(fp, ",msBetweenLsrs,AppMissed,LsrMissed");
    if (args.mTrackDebug) {
        fprintf(fp, ",msSourceReleaseFromRenderingToLsrAcquire,msAppCpuRenderFrame");
    }
    fprintf(fp, ",msAppPoseLatency");
    if (args.mTrackDebug) {
        fprintf(fp, ",msAppMisprediction,msLsrCpuRenderFrame");
    }
    fprintf(fp, ",msLsrPoseLatency,msActualLsrPoseLatency,msTimeUntilVsync,msLsrThreadWakeupToGpuEnd,msLsrThreadWakeupError");
    if (args.mTrackDebug) {
        fprintf(fp, ",msLsrThreadWakeupToCpuRenderFrameStart,msCpuRenderFrameStartToHeadPoseCallbackStart,msGetHeadPose,msHeadPoseCallbackStopToInputLatch,msInputLatchToGpuSubmission");
    }
    fprintf(fp, ",msLsrPreemption,msLsrExecution,msCopyPreemption,msCopyExecution,msGpuEndToVsync");
    fprintf(fp, "\n");

    return fp;
}

void UpdateLsrCsv(LateStageReprojectionData& lsr, ProcessInfo* proc, LateStageReprojectionEvent& p)
{
    auto const& args = GetCommandLineArgs();

    auto fp = GetOutputCsv(proc).mWmrFile;
    if (fp == nullptr) {
        return;
    }

    if (args.mExcludeDropped && p.FinalState != LateStageReprojectionResult::Presented) {
        return;
    }

    auto len = lsr.mLSRHistory.size();
    if (len < 2) {
        return;
    }

    auto& curr = lsr.mLSRHistory[len - 1];
    auto& prev = lsr.mLSRHistory[len - 2];
    const double deltaMilliseconds = 1000.0 * QpcDeltaToSeconds(curr.QpcTime - prev.QpcTime);
    const double timeInSeconds = QpcToSeconds(p.QpcTime);

    fprintf(fp, "%s,%d,%d", proc->mModuleName.c_str(), curr.GetAppProcessId(), curr.ProcessId);
    if (args.mTrackDebug) {
        fprintf(fp, ",%d", curr.GetAppFrameId());
    }
    fprintf(fp, ",%.6lf", timeInSeconds);
    if (args.mTrackDisplay) {
        double appPresentDeltaMilliseconds = 0.0;
        double appPresentToLsrMilliseconds = 0.0;
        if (curr.IsValidAppFrame()) {
            const uint64_t currAppPresentTime = curr.GetAppPresentTime();
            appPresentToLsrMilliseconds = 1000.0 * QpcDeltaToSeconds(curr.QpcTime - currAppPresentTime);

            if (prev.IsValidAppFrame() && (curr.GetAppProcessId() == prev.GetAppProcessId())) {
                const uint64_t prevAppPresentTime = prev.GetAppPresentTime();
                appPresentDeltaMilliseconds = 1000.0 * QpcDeltaToSeconds(currAppPresentTime - prevAppPresentTime);
            }
        }
        fprintf(fp, ",%.6lf,%.6lf", appPresentDeltaMilliseconds, appPresentToLsrMilliseconds);
    }
    fprintf(fp, ",%.6lf,%d,%d", deltaMilliseconds, !curr.NewSourceLatched, curr.MissedVsyncCount);
    if (args.mTrackDebug) {
        fprintf(fp, ",%.6lf,%.6lf", 1000 * QpcDeltaToSeconds(curr.Source.GetReleaseFromRenderingToAcquireForPresentationTime()), 1000.0 * QpcDeltaToSeconds(curr.GetAppCpuRenderFrameTime()));
    }
    fprintf(fp, ",%.6lf", curr.AppPredictionLatencyMs);
    if (args.mTrackDebug) {
        fprintf(fp, ",%.6lf,%.6lf", curr.AppMispredictionMs, curr.GetLsrCpuRenderFrameMs());
    }
    fprintf(fp, ",%.6lf,%.6lf,%.6lf,%.6lf,%.6lf",
        curr.LsrPredictionLatencyMs,
        curr.GetLsrMotionToPhotonLatencyMs(),
        curr.TimeUntilVsyncMs,
        curr.GetLsrThreadWakeupStartLatchToGpuEndMs(),
        curr.TotalWakeupErrorMs);
    if (args.mTrackDebug) {
        fprintf(fp, ",%.6lf,%.6lf,%.6lf,%.6lf,%.6lf",
            curr.ThreadWakeupStartLatchToCpuRenderFrameStartInMs,
            curr.CpuRenderFrameStartToHeadPoseCallbackStartInMs,
            curr.HeadPoseCallbackStartToHeadPoseCallbackStopInMs,
            curr.HeadPoseCallbackStopToInputLatchInMs,
            curr.InputLatchToGpuSubmissionInMs);
    }
    fprintf(fp, ",%.6lf,%.6lf,%.6lf,%.6lf,%.6lf",
        curr.GpuSubmissionToGpuStartInMs,
        curr.GpuStartToGpuStopInMs,
        curr.GpuStopToCopyStartInMs,
        curr.CopyStartToCopyStopInMs,
        curr.CopyStopToVsyncInMs);
    fprintf(fp, "\n");
}

void UpdateConsole(std::unordered_map<uint32_t, ProcessInfo> const& activeProcesses, LateStageReprojectionData& lsr)
{
    auto const& args = GetCommandLineArgs();

    // LSR info
    if (lsr.HasData()) {
        ConsolePrintLn("");
        ConsolePrintLn("Windows Mixed Reality:");

        const LateStageReprojectionRuntimeStats runtimeStats = lsr.ComputeRuntimeStats();
        const double historyTime = lsr.ComputeHistoryTime();

        {
            // App
            const double fps = lsr.ComputeSourceFps();
            const size_t historySize = lsr.ComputeHistorySize();

            if (args.mTrackDisplay) {
                auto processIter = activeProcesses.find(runtimeStats.mAppProcessId);
                ConsolePrintLn("    App - %s[%d]:",
                    processIter == activeProcesses.end() ? "<error>" : processIter->second.mModuleName.c_str(),
                    runtimeStats.mAppProcessId);
                ConsolePrint("        %.2lf ms/frame (%.1lf fps, %.2lf ms CPU", 1000.0 / fps, fps, runtimeStats.mAppSourceCpuRenderTimeInMs);
            } else {
                ConsolePrintLn("    App:");
                ConsolePrint("        %.2lf ms/frame (%.1lf fps", 1000.0 / fps, fps);
            }

            ConsolePrintLn(", %.1lf%% of Compositor frame rate)", double(historySize - runtimeStats.mAppMissedFrames) / (historySize) * 100.0f);

            ConsolePrintLn("        Missed Present: %Iu total in last %.1lf seconds (%Iu total observed)",
                runtimeStats.mAppMissedFrames,
                historyTime,
                lsr.mLifetimeAppMissedFrames);

            ConsolePrintLn("        Post-Present to Compositor CPU: %.2lf ms",
                runtimeStats.mAppSourceReleaseToLsrAcquireInMs);
        }

        {
            // LSR
            const double fps = lsr.ComputeFps();
            auto processIter = activeProcesses.find(runtimeStats.mLsrProcessId);

            ConsolePrintLn("    Compositor - %s[%d]:",
                processIter == activeProcesses.end() ? "<error>" : processIter->second.mModuleName.c_str(),
                runtimeStats.mLsrProcessId);
            ConsolePrintLn("        %.2lf ms/frame (%.1lf fps, %.1lf displayed fps, %.2lf ms CPU)",
                1000.0 / fps,
                fps,
                lsr.ComputeDisplayedFps(),
                runtimeStats.mLsrCpuRenderTimeInMs);

            ConsolePrintLn("        Missed V-Sync: %Iu consecutive, %Iu total in last %.1lf seconds (%Iu total observed)",
                runtimeStats.mLsrConsecutiveMissedFrames,
                runtimeStats.mLsrMissedFrames,
                historyTime,
                lsr.mLifetimeLsrMissedFrames);

            ConsolePrintLn("        Reprojection: %.2lf ms gpu preemption (%.2lf ms max) | %.2lf ms gpu execution (%.2lf ms max)",
                runtimeStats.mGpuPreemptionInMs.GetAverage(),
                runtimeStats.mGpuPreemptionInMs.GetMax(),
                runtimeStats.mGpuExecutionInMs.GetAverage(),
                runtimeStats.mGpuExecutionInMs.GetMax());

            if (runtimeStats.mCopyExecutionInMs.GetAverage() > 0.0) {
                ConsolePrintLn("        Hybrid Copy: %.2lf ms gpu preemption (%.2lf ms max) | %.2lf ms gpu execution (%.2lf ms max)",
                    runtimeStats.mCopyPreemptionInMs.GetAverage(),
                    runtimeStats.mCopyPreemptionInMs.GetMax(),
                    runtimeStats.mCopyExecutionInMs.GetAverage(),
                    runtimeStats.mCopyExecutionInMs.GetMax());
            }

            ConsolePrintLn("        Gpu-End to V-Sync: %.2lf ms", runtimeStats.mGpuEndToVsyncInMs);
        }

        {
            // Latency
            ConsolePrintLn("    Pose Latency:");
            ConsolePrintLn("        App Motion-to-Mid-Photon: %.2lf ms", runtimeStats.mAppPoseLatencyInMs);
            ConsolePrintLn("        Compositor Motion-to-Mid-Photon: %.2lf ms (%.2lf ms to V-Sync)",
                runtimeStats.mLsrPoseLatencyInMs,
                runtimeStats.mLsrInputLatchToVsyncInMs.GetAverage());
            ConsolePrintLn("        V-Sync to Mid-Photon: %.2lf ms", runtimeStats.mVsyncToPhotonsMiddleInMs);
        }

        ConsolePrintLn("");
    }
}
