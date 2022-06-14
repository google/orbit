// Copyright (C) 2017,2019-2022 Intel Corporation
// SPDX-License-Identifier: MIT

#include "PresentMon.hpp"

#include "../PresentData/TraceSession.hpp"

namespace {

TraceSession gSession;
static PMTraceConsumer* gPMConsumer = nullptr;
static MRTraceConsumer* gMRConsumer = nullptr;

}

bool StartTraceSession()
{
    auto const& args = GetCommandLineArgs();
    auto filterProcessIds = args.mTargetPid != 0; // Does not support process names at this point

    // Create consumers
    gPMConsumer = new PMTraceConsumer();
    gPMConsumer->mFilteredProcessIds = filterProcessIds;
    gPMConsumer->mTrackDisplay = args.mTrackDisplay;

    if (filterProcessIds) {
        gPMConsumer->AddTrackedProcessForFiltering(args.mTargetPid);
    }

    if (args.mTrackWMR) {
        gMRConsumer = new MRTraceConsumer(args.mTrackDisplay);
    }

    // Start the session;
    // If a session with this same name is already running, we either exit or
    // stop it and start a new session.  This is useful if a previous process
    // failed to properly shut down the session for some reason.
    auto status = gSession.Start(gPMConsumer, gMRConsumer, args.mEtlFileName, args.mSessionName);

    if (status == ERROR_ALREADY_EXISTS) {
        if (args.mStopExistingSession) {
            PrintWarning(
                "warning: a trace session named \"%s\" is already running and it will be stopped.\n"
                "         Use -session_name with a different name to start a new session.\n",
                args.mSessionName);
        } else {
            PrintError(
                "error: a trace session named \"%s\" is already running. Use -stop_existing_session\n"
                "       to stop the existing session, or use -session_name with a different name to\n"
                "       start a new session.\n",
                args.mSessionName);
            delete gPMConsumer;
            delete gMRConsumer;
            gPMConsumer = nullptr;
            gMRConsumer = nullptr;
            return false;
        }

        status = TraceSession::StopNamedSession(args.mSessionName);
        if (status == ERROR_SUCCESS) {
            status = gSession.Start(gPMConsumer, gMRConsumer, args.mEtlFileName, args.mSessionName);
        }
    }

    // Report error if we failed to start a new session
    if (status != ERROR_SUCCESS) {
        PrintError("error: failed to start trace session");
        switch (status) {
        case ERROR_FILE_NOT_FOUND:    PrintError(" (file not found)"); break;
        case ERROR_PATH_NOT_FOUND:    PrintError(" (path not found)"); break;
        case ERROR_BAD_PATHNAME:      PrintError(" (invalid --session_name)"); break;
        case ERROR_ACCESS_DENIED:     PrintError(" (access denied)"); break;
        case ERROR_FILE_CORRUPT:      PrintError(" (invalid --etl_file)"); break;
        default:                      PrintError(" (error=%lu)", status); break;
        }
        PrintError(".\n");

        if (status == ERROR_ACCESS_DENIED && !InPerfLogUsersGroup()) {
            PrintError(
                "       PresentMon requires either administrative privileges or to be run by a user in the\n"
                "       \"Performance Log Users\" user group.  View the readme for more details.\n");
        }

        delete gPMConsumer;
        delete gMRConsumer;
        gPMConsumer = nullptr;
        gMRConsumer = nullptr;
        return false;
    }

    // -------------------------------------------------------------------------
    // Start the consumer and output threads
    StartConsumerThread(gSession.mTraceHandle);
    StartOutputThread();

    return true;
}

void StopTraceSession()
{
    // Stop the trace session.
    gSession.Stop();

    // Wait for the consumer and output threads to end (which are using the
    // consumers).
    WaitForConsumerThreadToExit();
    StopOutputThread();

    // Destruct the consumers
    delete gMRConsumer;
    delete gPMConsumer;
    gMRConsumer = nullptr;
    gPMConsumer = nullptr;
}

void CheckLostReports(ULONG* eventsLost, ULONG* buffersLost)
{
    auto status = gSession.CheckLostReports(eventsLost, buffersLost);
    (void) status;
}

void DequeueAnalyzedInfo(
    std::vector<ProcessEvent>* processEvents,
    std::vector<std::shared_ptr<PresentEvent>>* presentEvents,
    std::vector<std::shared_ptr<PresentEvent>>* lostPresentEvents,
    std::vector<std::shared_ptr<LateStageReprojectionEvent>>* lsrs)
{
    gPMConsumer->DequeueProcessEvents(*processEvents);
    gPMConsumer->DequeuePresentEvents(*presentEvents);
    gPMConsumer->DequeueLostPresentEvents(*lostPresentEvents);
    if (gMRConsumer != nullptr) {
        gMRConsumer->DequeueLSRs(*lsrs);
    }
}

double QpcDeltaToSeconds(uint64_t qpcDelta)
{
    return (double) qpcDelta / gSession.mQpcFrequency.QuadPart;
}

uint64_t SecondsDeltaToQpc(double secondsDelta)
{
    return (uint64_t) (secondsDelta * gSession.mQpcFrequency.QuadPart);
}

double QpcToSeconds(uint64_t qpc)
{
    return QpcDeltaToSeconds(qpc - gSession.mStartQpc.QuadPart);
}
