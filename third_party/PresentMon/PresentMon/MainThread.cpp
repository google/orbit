// Copyright (C) 2019-2022 Intel Corporation
// SPDX-License-Identifier: MIT

#include "PresentMon.hpp"
#include "../PresentData/TraceSession.hpp"

enum {
    HOTKEY_ID = 0x80,

    // Timer ID's must be non-zero
    DELAY_TIMER_ID = 1,
    TIMED_TIMER_ID = 2,
};

static HWND gWnd = NULL;
static bool gIsRecording = false;
static uint32_t gHotkeyIgnoreCount = 0;

static bool EnableScrollLock(bool enable)
{
    auto const& args = GetCommandLineArgs();

    auto enabled = (GetKeyState(VK_SCROLL) & 1) == 1;
    if (enabled != enable) {
        // If the hotkey is SCROLLLOCK, SendInput() will cause the hotkey to
        // trigger (entering an infinite recording toggle loop) so note that
        // the message handler should ignore one of them.
        if (args.mHotkeySupport &&
            args.mHotkeyVirtualKeyCode == VK_SCROLL &&
            args.mHotkeyModifiers == MOD_NOREPEAT) {
            gHotkeyIgnoreCount += 1;
        }

        // Send SCROLLLOCK press message.
        auto extraInfo = GetMessageExtraInfo();
        INPUT input[2] = {};

        input[0].type = INPUT_KEYBOARD;
        input[0].ki.wVk = VK_SCROLL;
        input[0].ki.dwExtraInfo = extraInfo;

        input[1].type = INPUT_KEYBOARD;
        input[1].ki.wVk = VK_SCROLL;
        input[1].ki.dwFlags = KEYEVENTF_KEYUP;
        input[1].ki.dwExtraInfo = extraInfo;

        auto sendCount = SendInput(2, input, sizeof(INPUT));
        if (sendCount != 2) {
            PrintWarning("warning: could not toggle scroll lock.\n");
        }
    }

    return enabled;
}

static bool IsRecording()
{
    return gIsRecording;
}

static void StartRecording()
{
    auto const& args = GetCommandLineArgs();

    assert(IsRecording() == false);
    gIsRecording = true;

    // Notify user we're recording
#if !DEBUG_VERBOSE
    if (args.mConsoleOutputType == ConsoleOutput::Simple) {
        printf("Started recording.\n");
    }
#endif
    if (args.mScrollLockIndicator) {
        EnableScrollLock(true);
    }

    // Tell OutputThread to record
    SetOutputRecordingState(true);

    // Start -timed timer
    if (args.mStartTimer) {
        SetTimer(gWnd, TIMED_TIMER_ID, args.mTimer * 1000, (TIMERPROC) nullptr);
    }
}

static void StopRecording()
{
    auto const& args = GetCommandLineArgs();

    assert(IsRecording() == true);
    gIsRecording = false;

    // Stop time -timed timer if there is one
    if (args.mStartTimer) {
        KillTimer(gWnd, TIMED_TIMER_ID);
    }

    // Tell OutputThread to stop recording
    SetOutputRecordingState(false);

    // Notify the user we're no longer recording
    if (args.mScrollLockIndicator) {
        EnableScrollLock(false);
    }
#if !DEBUG_VERBOSE
    if (args.mConsoleOutputType == ConsoleOutput::Simple) {
        printf("Stopped recording.\n");
    }
#endif
}

// Handle Ctrl events (CTRL_C_EVENT, CTRL_BREAK_EVENT, CTRL_CLOSE_EVENT,
// CTRL_LOGOFF_EVENT, CTRL_SHUTDOWN_EVENT) by redirecting the termination into
// a WM_QUIT message so that the shutdown code is still executed.
static BOOL CALLBACK HandleCtrlEvent(DWORD ctrlType)
{
    (void) ctrlType;
    if (IsRecording()) {
        StopRecording();
    }
    ExitMainThread();

    // The other threads are now shutting down but if we return the system may
    // terminate the process before they complete, which may leave the trace
    // session open.  We could wait for shutdown confirmation, but this
    // function is run in a separate thread so we just put it to sleep
    // indefinately and let the application shut itself down.
    Sleep(INFINITE);
    return TRUE; // The signal was handled, don't call any other handlers
}

// Handle window messages to toggle recording on/off
static LRESULT CALLBACK HandleWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    auto const& args = GetCommandLineArgs();

    switch (uMsg) {
    case WM_TIMER:
        switch (wParam) {
        case DELAY_TIMER_ID:
            StartRecording();
            KillTimer(hWnd, DELAY_TIMER_ID);
            return 0;

        case TIMED_TIMER_ID:
            StopRecording();
            if (args.mTerminateAfterTimer) {
                ExitMainThread();
            }
            return 0;
        }
        break;

    case WM_HOTKEY:
        if (gHotkeyIgnoreCount > 0) {
            gHotkeyIgnoreCount -= 1;
            break;
        }

        if (IsRecording()) {
            StopRecording();
        } else if (args.mDelay == 0) {
            StartRecording();
        } else {
            SetTimer(hWnd, DELAY_TIMER_ID, args.mDelay * 1000, (TIMERPROC) nullptr);
        }
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void ExitMainThread()
{
    PostMessage(gWnd, WM_QUIT, 0, 0);
}

int main(int argc, char** argv)
{
    // Load system DLLs
    LoadLibraryExA("advapi32.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    LoadLibraryExA("shell32.dll",  NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    LoadLibraryExA("shlwapi.dll",  NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    LoadLibraryExA("tdh.dll",      NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    LoadLibraryExA("user32.dll",   NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);

    // Initialize console
    InitializeConsole();

    // Parse command line arguments.
    if (!ParseCommandLine(argc, argv)) {
        return 1;
    }

    auto const& args = GetCommandLineArgs();

    // Special case handling for -terminate_existing
    if (args.mTerminateExisting) {
        auto status = TraceSession::StopNamedSession(args.mSessionName);
        switch (status) {
        case ERROR_SUCCESS: return 0;
        case ERROR_WMI_INSTANCE_NOT_FOUND: PrintError("error: no existing sessions found: %s\n", args.mSessionName); break;
        default: PrintError("error: failed to terminate existing session (%s): %lu\n", args.mSessionName, status); break;
        }
        return 7;
    }

    // Attempt to elevate process privilege if necessary.
    //
    // If we are processing an ETL file we don't need elevated privilege, but
    // for realtime analysis we need SeDebugPrivilege in order to open handles
    // to processes started by other accounts (see OutputThread.cpp).
    // 
    // If we can't enable SeDebugPrivilege, try to restart PresentMon as
    // administrator unless the user requested not to.
    // 
    // RestartAsAdministrator() waits for the elevated process to complete in
    // order to report stderr and obtain it's exit code.
    if (args.mEtlFileName == nullptr && // realtime analysis
        !EnableDebugPrivilege()) {      // failed to enable SeDebugPrivilege
        if (args.mTryToElevate) {
            return RestartAsAdministrator(argc, argv);
        }

        PrintWarning(
            "warning: PresentMon requires elevated privilege in order to query processes started\n"
            "    on another account.  Without it, those processes will be listed as '<error>'\n"
            "    and they can't be targeted by -process_name nor trigger -terminate_on_proc_exit.\n");
    }

    // Create a message queue to handle the input messages.
    WNDCLASSEXW wndClass = { sizeof(wndClass) };
    wndClass.lpfnWndProc = HandleWindowMessage;
    wndClass.lpszClassName = L"PresentMon";
    if (!RegisterClassExW(&wndClass)) {
        PrintError("error: failed to register hotkey class.\n");
        return 3;
    }

    gWnd = CreateWindowExW(0, wndClass.lpszClassName, L"PresentMonWnd", 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, nullptr);
    if (!gWnd) {
        PrintError("error: failed to create hotkey window.\n");
        UnregisterClass(wndClass.lpszClassName, NULL);
        return 4;
    }

    // Register the hotkey.
    if (args.mHotkeySupport && !RegisterHotKey(gWnd, HOTKEY_ID, args.mHotkeyModifiers, args.mHotkeyVirtualKeyCode)) {
        PrintError("error: failed to register hotkey.\n");
        DestroyWindow(gWnd);
        UnregisterClass(wndClass.lpszClassName, NULL);
        return 5;
    }

    // Set CTRL handler (note: must set gWnd before setting the handler).
    SetConsoleCtrlHandler(HandleCtrlEvent, TRUE);

    // Start the ETW trace session (including consumer and output threads).
    if (!StartTraceSession()) {
        SetConsoleCtrlHandler(HandleCtrlEvent, FALSE);
        DestroyWindow(gWnd);
        UnregisterClass(wndClass.lpszClassName, NULL);
        return 6;
    }

    // If the user wants to use the scroll lock key as an indicator of when
    // PresentMon is recording events, save the original state and set scroll
    // lock to the recording state.
    auto originalScrollLockEnabled = args.mScrollLockIndicator
        ? EnableScrollLock(IsRecording())
        : false;

    // If the user didn't specify -hotkey, simulate a hotkey press to start the
    // recording right away.
    if (!args.mHotkeySupport) {
        PostMessage(gWnd, WM_HOTKEY, HOTKEY_ID, args.mHotkeyModifiers & ~MOD_NOREPEAT);
    }

    // Enter the MainThread message loop.  This thread will block waiting for
    // any window messages, dispatching the appropriate function to
    // HandleWindowMessage(), and then blocking again until the WM_QUIT message
    // arrives or the window is destroyed.
    for (MSG message = {};;) {
        BOOL r = GetMessageW(&message, gWnd, 0, 0);
        if (r == 0) { // Received WM_QUIT message.
            break;
        }
        if (r == -1) { // Indicates error in message loop, e.g. gWnd is no
                       // longer valid. This can happen if PresentMon is killed.
            break;
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    // Shut everything down.
    if (args.mScrollLockIndicator) {
        EnableScrollLock(originalScrollLockEnabled);
    }
    StopTraceSession();
    /* We cannot remove the Ctrl handler because it is in an infinite sleep so
     * this call will never return, either hanging the application or having
     * the threshold timer trigger and force terminate (depending on what Ctrl
     * code was used).  Instead, we just let the process tear down take care of
     * it.
    SetConsoleCtrlHandler(HandleCtrlEvent, FALSE);
    */
    DestroyWindow(gWnd);
    UnregisterClass(wndClass.lpszClassName, NULL);
    return 0;
}
