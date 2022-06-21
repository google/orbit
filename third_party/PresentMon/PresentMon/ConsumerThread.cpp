// Copyright (C) 2019-2021 Intel Corporation
// SPDX-License-Identifier: MIT

#include "PresentMon.hpp"

static std::thread gThread;

static void Consume(TRACEHANDLE traceHandle)
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    // You must call OpenTrace() prior to calling this function
    //
    // ProcessTrace() blocks the calling thread until it
    //     1) delivers all events in a trace log file, or
    //     2) the BufferCallback function returns FALSE, or
    //     3) you call CloseTrace(), or
    //     4) the controller stops the trace session.
    //
    // There may be a several second delay before the function returns.
    //
    // ProcessTrace() is supposed to return ERROR_CANCELLED if BufferCallback
    // (EtwThreadsShouldQuit) returns FALSE; and ERROR_SUCCESS if the trace
    // completes (parses the entire ETL, fills the maximum file size, or is
    // explicitly closed).
    //
    // However, it seems to always return ERROR_SUCCESS.

    auto status = ProcessTrace(&traceHandle, 1, NULL, NULL);
    (void) status;

    // Signal MainThread to exit.  This is only needed if we are processing an
    // ETL file and ProcessTrace() returned because the ETL is done, but there
    // is no harm in calling ExitMainThread() if MainThread is already exiting
    // (and caused ProcessTrace() to exit via 2, 3, or 4 above) because the
    // message queue isn't beeing listened too anymore in that case.
    ExitMainThread();
}

void StartConsumerThread(TRACEHANDLE traceHandle)
{
    gThread = std::thread(Consume, traceHandle);
}

void WaitForConsumerThreadToExit()
{
    if (gThread.joinable()) {
        gThread.join();
    }
}
