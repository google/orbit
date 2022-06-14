// Copyright (C) 2019-2021 Intel Corporation
// SPDX-License-Identifier: MIT

#include "PresentMon.hpp"

static HANDLE gConsoleHandle = INVALID_HANDLE_VALUE;
static char gConsoleWriteBuffer[8 * 1024] = {};
static uint32_t gConsoleWriteBufferIndex = 0;
static uint32_t gConsolePrevWriteBufferSize = 0;
static SHORT gConsoleTop = 0;
static SHORT gConsoleWidth = 0;
static SHORT gConsoleBufferHeight = 0;
static bool gConsoleFirstCommit = true;

bool IsConsoleInitialized()
{
    return gConsoleHandle != INVALID_HANDLE_VALUE;
}

bool InitializeConsole()
{
    if (!IsConsoleInitialized()) {
        gConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (gConsoleHandle == INVALID_HANDLE_VALUE) {
            return false;
        }

        CONSOLE_SCREEN_BUFFER_INFO info = {};
        if (GetConsoleScreenBufferInfo(gConsoleHandle, &info) == 0) {
            gConsoleHandle = INVALID_HANDLE_VALUE;
            return false;
        }

        gConsoleTop = info.dwCursorPosition.Y;
        gConsoleWidth = info.srWindow.Right - info.srWindow.Left + 1;
        gConsoleBufferHeight = info.dwSize.Y;
        gConsoleWriteBufferIndex = 0;
        gConsolePrevWriteBufferSize = 0;
        gConsoleFirstCommit = true;
    }

    return true;
}

static void vConsolePrint(char const* format, va_list args)
{
    auto s = gConsoleWriteBuffer + gConsoleWriteBufferIndex;
    auto n = sizeof(gConsoleWriteBuffer) - gConsoleWriteBufferIndex;

    int r = vsnprintf(s, n, format, args);
    if (r > 0) {
        gConsoleWriteBufferIndex = min((uint32_t) (n - 1), gConsoleWriteBufferIndex + r);
    }
}

void ConsolePrint(char const* format, ...)
{
    va_list args;
    va_start(args, format);
    vConsolePrint(format, args);
    va_end(args);
}

void ConsolePrintLn(char const* format, ...)
{
    va_list args;
    va_start(args, format);
    vConsolePrint(format, args);
    va_end(args);

    auto x = gConsoleWriteBufferIndex % gConsoleWidth;
    auto s = gConsoleWidth - x;
    memset(gConsoleWriteBuffer + gConsoleWriteBufferIndex, ' ', s);
    gConsoleWriteBufferIndex += s;
}

void CommitConsole()
{
    auto sizeWritten = gConsoleWriteBufferIndex;
    auto linesWritten = (SHORT) (sizeWritten / gConsoleWidth);

    // Reset gConsoleTop on the first commit so we don't overwrite any warning
    // messages.
    auto size = sizeWritten;
    if (gConsoleFirstCommit) {
        gConsoleFirstCommit = false;

        CONSOLE_SCREEN_BUFFER_INFO info = {};
        GetConsoleScreenBufferInfo(gConsoleHandle, &info);
        gConsoleTop = info.dwCursorPosition.Y;
    } else {
        // Write some extra empty lines to make sure we clear anything from
        // last time.
        if (size < gConsolePrevWriteBufferSize) {
            memset(gConsoleWriteBuffer + size, ' ', gConsolePrevWriteBufferSize - size);
            size = gConsolePrevWriteBufferSize;
        }
    }

    // If we're at the end of the console buffer, issue some new lines to make
    // some space.
    auto maxCursorY = gConsoleBufferHeight - linesWritten;
    if (gConsoleTop > maxCursorY) {
        COORD bottom = { 0, gConsoleBufferHeight - 1 };
        SetConsoleCursorPosition(gConsoleHandle, bottom);
        printf("\n");
        for (--gConsoleTop; gConsoleTop > maxCursorY; --gConsoleTop) {
            printf("\n");
        }
    }

    // Write to the console.
    DWORD dwCharsWritten = 0;
    COORD cursor = { 0, gConsoleTop };
    WriteConsoleOutputCharacterA(gConsoleHandle, gConsoleWriteBuffer, (DWORD) size, cursor, &dwCharsWritten);

    // Put the cursor at the end of the written text.
    cursor.Y += linesWritten;
    SetConsoleCursorPosition(gConsoleHandle, cursor);

    // Update console info in case it was resized.
    CONSOLE_SCREEN_BUFFER_INFO info = {};
    GetConsoleScreenBufferInfo(gConsoleHandle, &info);
    gConsoleWidth = info.srWindow.Right - info.srWindow.Left + 1;
    gConsoleBufferHeight = info.dwSize.Y;
    gConsoleWriteBufferIndex = 0;
    gConsolePrevWriteBufferSize = sizeWritten;
}

void UpdateConsole(uint32_t processId, ProcessInfo const& processInfo)
{
    auto const& args = GetCommandLineArgs();

    // Don't display non-target or empty processes
    if (!processInfo.mTargetProcess ||
        processInfo.mModuleName.empty() ||
        processInfo.mSwapChain.empty()) {
        return;
    }

    auto empty = true;

    for (auto const& pair : processInfo.mSwapChain) {
        auto address = pair.first;
        auto const& chain = pair.second;

        // Only show swapchain data if there at least two presents in the
        // history.
        if (chain.mPresentHistoryCount < 2) {
            continue;
        }

        auto const& present0 = *chain.mPresentHistory[(chain.mNextPresentIndex - chain.mPresentHistoryCount) % SwapChainData::PRESENT_HISTORY_MAX_COUNT];
        auto const& presentN = *chain.mPresentHistory[(chain.mNextPresentIndex - 1) % SwapChainData::PRESENT_HISTORY_MAX_COUNT];
        auto cpuAvg = QpcDeltaToSeconds(presentN.QpcTime - present0.QpcTime) / (chain.mPresentHistoryCount - 1);
        auto dspAvg = 0.0;
        auto latAvg = 0.0;

        PresentEvent* displayN = nullptr;
        if (args.mTrackDisplay) {
            uint64_t display0ScreenTime = 0;
            uint64_t latSum = 0;
            uint32_t displayCount = 0;
            for (uint32_t i = 0; i < chain.mPresentHistoryCount; ++i) {
                auto const& p = chain.mPresentHistory[(chain.mNextPresentIndex - chain.mPresentHistoryCount + i) % SwapChainData::PRESENT_HISTORY_MAX_COUNT];
                if (p->FinalState == PresentResult::Presented) {
                    if (displayCount == 0) {
                        display0ScreenTime = p->ScreenTime;
                    }
                    displayN = p.get();
                    latSum += p->ScreenTime - p->QpcTime;
                    displayCount += 1;
                }
            }

            if (displayCount >= 2) {
                dspAvg = QpcDeltaToSeconds(displayN->ScreenTime - display0ScreenTime) / (displayCount - 1);
            }

            if (displayCount >= 1) {
                latAvg = QpcDeltaToSeconds(latSum) / displayCount;
            }
        }

        if (empty) {
            empty = false;
            ConsolePrintLn("%s[%d]:", processInfo.mModuleName.c_str(), processId);
        }

        ConsolePrint("    %016llX (%s): SyncInterval=%d Flags=%d CPU%s=%.2lf",
            address,
            RuntimeToString(presentN.Runtime),
            presentN.SyncInterval,
            presentN.PresentFlags,
            dspAvg > 0.0 ? "/Display" : "",
            1000.0 * cpuAvg);

        if (dspAvg > 0.0) ConsolePrint("/%.2lf", 1000.0 * dspAvg);

        ConsolePrint("ms (%.1lf", 1.0 / cpuAvg);
        if (dspAvg > 0.0) ConsolePrint("/%.1lf", 1.0 / dspAvg);
        ConsolePrint(" fps)");

        if (latAvg > 0.0) {
            ConsolePrint(" latency=%.2lfms", 1000.0 * latAvg);
        }

        if (displayN != nullptr) {
            ConsolePrint(" %s", PresentModeToString(displayN->PresentMode));
        }

        ConsolePrintLn("");
    }

    if (!empty) {
        ConsolePrintLn("");
    }
}

namespace {

int PrintColor(WORD color, char const* format, va_list val)
{
    #ifndef NDEBUG
    {
        char buffer[256];
        if (_vsnprintf_s(buffer, _TRUNCATE, format, val) == -1) {
            buffer[252] = '.';
            buffer[253] = '.';
            buffer[254] = '.';
        }
        OutputDebugStringA(buffer);
    }
    #endif

    CONSOLE_SCREEN_BUFFER_INFO info = {};
    auto setColor = IsConsoleInitialized() && GetConsoleScreenBufferInfo(gConsoleHandle, &info) != 0;
    if (setColor) {
        auto bg = info.wAttributes & (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY);
        if (bg == 0) {
            color |= FOREGROUND_INTENSITY;
        }
        SetConsoleTextAttribute(gConsoleHandle, WORD(bg | color));
    }

    int c = vfprintf(stderr, format, val);

    if (setColor) {
        SetConsoleTextAttribute(gConsoleHandle, info.wAttributes);
    }

    return c;
}

}

int PrintWarning(char const* format, ...)
{
    va_list val;
    va_start(val, format);
    int c = PrintColor(FOREGROUND_RED | FOREGROUND_GREEN, format, val);
    va_end(val);
    return c;
}

int PrintError(char const* format, ...)
{
    va_list val;
    va_start(val, format);
    int c = PrintColor(FOREGROUND_RED, format, val);
    va_end(val);
    return c;
}
