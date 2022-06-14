// Copyright (C) 2017,2019-2022 Intel Corporation
// SPDX-License-Identifier: MIT

#include <generated/version.h>

#define NOMINMAX
#include "PresentMon.hpp"
#include <algorithm>

namespace {

enum {
    DEFAULT_CONSOLE_WIDTH   = 80,
    MAX_ARG_COLUMN_WIDTH    = 40,
    MIN_DESC_COLUMN_WIDTH   = 20,
    ARG_DESC_COLUMN_PADDING = 4,
};

struct KeyNameCode
{
    char const* mName;
    UINT mCode;
};

KeyNameCode const HOTKEY_MODS[] = {
    { "ALT",     MOD_ALT     },
    { "CONTROL", MOD_CONTROL },
    { "CTRL",    MOD_CONTROL },
    { "SHIFT",   MOD_SHIFT   },
    { "WINDOWS", MOD_WIN     },
    { "WIN",     MOD_WIN     },
};

KeyNameCode const HOTKEY_KEYS[] = {
    { "BACKSPACE", VK_BACK },
    { "TAB", VK_TAB },
    { "CLEAR", VK_CLEAR },
    { "ENTER", VK_RETURN },
    { "PAUSE", VK_PAUSE },
    { "CAPSLOCK", VK_CAPITAL },
    { "ESC", VK_ESCAPE },
    { "SPACE", VK_SPACE },
    { "PAGEUP", VK_PRIOR },
    { "PAGEDOWN", VK_NEXT },
    { "END", VK_END },
    { "HOME", VK_HOME },
    { "LEFT", VK_LEFT },
    { "UP", VK_UP },
    { "RIGHT", VK_RIGHT },
    { "DOWN", VK_DOWN },
    { "PRINTSCREEN", VK_SNAPSHOT },
    { "INS", VK_INSERT },
    { "DEL", VK_DELETE },
    { "HELP", VK_HELP },
    { "NUMLOCK", VK_NUMLOCK },
    { "SCROLLLOCK", VK_SCROLL },
    { "NUM0", VK_NUMPAD0 },
    { "NUM1", VK_NUMPAD1 },
    { "NUM2", VK_NUMPAD2 },
    { "NUM3", VK_NUMPAD3 },
    { "NUM4", VK_NUMPAD4 },
    { "NUM5", VK_NUMPAD5 },
    { "NUM6", VK_NUMPAD6 },
    { "NUM7", VK_NUMPAD7 },
    { "NUM8", VK_NUMPAD8 },
    { "NUM9", VK_NUMPAD9 },
    { "MULTIPLY", VK_MULTIPLY },
    { "ADD", VK_ADD },
    { "SEPARATOR", VK_SEPARATOR },
    { "SUBTRACT", VK_SUBTRACT },
    { "DECIMAL", VK_DECIMAL },
    { "DIVIDE", VK_DIVIDE },
    { "0", 0x30 },
    { "1", 0x31 },
    { "2", 0x32 },
    { "3", 0x33 },
    { "4", 0x34 },
    { "5", 0x35 },
    { "6", 0x36 },
    { "7", 0x37 },
    { "8", 0x38 },
    { "9", 0x39 },
    { "A", 0x41 },
    { "B", 0x42 },
    { "C", 0x43 },
    { "D", 0x44 },
    { "E", 0x45 },
    { "F", 0x46 },
    { "G", 0x47 },
    { "H", 0x48 },
    { "I", 0x49 },
    { "J", 0x4A },
    { "K", 0x4B },
    { "L", 0x4C },
    { "M", 0x4D },
    { "N", 0x4E },
    { "O", 0x4F },
    { "P", 0x50 },
    { "Q", 0x51 },
    { "R", 0x52 },
    { "S", 0x53 },
    { "T", 0x54 },
    { "U", 0x55 },
    { "V", 0x56 },
    { "W", 0x57 },
    { "X", 0x58 },
    { "Y", 0x59 },
    { "Z", 0x5A },
    { "F1", VK_F1 },
    { "F2", VK_F2 },
    { "F3", VK_F3 },
    { "F4", VK_F4 },
    { "F5", VK_F5 },
    { "F6", VK_F6 },
    { "F7", VK_F7 },
    { "F8", VK_F8 },
    { "F9", VK_F9 },
    { "F10", VK_F10 },
    { "F11", VK_F11 },
    { "F12", VK_F12 },
    { "F13", VK_F13 },
    { "F14", VK_F14 },
    { "F15", VK_F15 },
    { "F16", VK_F16 },
    { "F17", VK_F17 },
    { "F18", VK_F18 },
    { "F19", VK_F19 },
    { "F20", VK_F20 },
    { "F21", VK_F21 },
    { "F22", VK_F22 },
    { "F23", VK_F23 },
    { "F24", VK_F24 },
};

CommandLineArgs gCommandLineArgs;

size_t GetConsoleWidth()
{
    CONSOLE_SCREEN_BUFFER_INFO info = {};
    return GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info) == 0
        ? DEFAULT_CONSOLE_WIDTH
        : std::max<size_t>(DEFAULT_CONSOLE_WIDTH, info.srWindow.Right - info.srWindow.Left + 1);
}

bool ParseKeyName(KeyNameCode const* valid, size_t validCount, char* name, char const* errorMessage, UINT* outKeyCode)
{
    for (size_t i = 0; i < validCount; ++i) {
        if (_stricmp(name, valid[i].mName) == 0) {
            *outKeyCode = valid[i].mCode;
            return true;
        }
    }

    int col = PrintError("error: %s '%s'.\nValid options (case insensitive):", errorMessage, name);

    size_t consoleWidth = GetConsoleWidth();
    for (size_t i = 0; i < validCount; ++i) {
        auto len = strlen(valid[i].mName);
        if (col + len + 1 > consoleWidth) {
            col = PrintError("\n   ") - 1;
        }
        col += PrintError(" %s", valid[i].mName);
    }
    PrintError("\n");

    return false;
}

bool AssignHotkey(char* key, CommandLineArgs* args)
{
#pragma warning(suppress: 4996)
    auto token = strtok(key, "+");
    for (;;) {
        auto prev = token;
#pragma warning(suppress: 4996)
        token = strtok(nullptr, "+");
        if (token == nullptr) {
            if (!ParseKeyName(HOTKEY_KEYS, _countof(HOTKEY_KEYS), prev, "invalid -hotkey key", &args->mHotkeyVirtualKeyCode)) {
                return false;
            }
            break;
        }

        if (!ParseKeyName(HOTKEY_MODS, _countof(HOTKEY_MODS), prev, "invalid -hotkey modifier", &args->mHotkeyModifiers)) {
            return false;
        }
    }

    args->mHotkeySupport = true;
    return true;
}

void SetCaptureAll(CommandLineArgs* args)
{
    if (!args->mTargetProcessNames.empty()) {
        PrintWarning("warning: -captureall elides all previous -process_name arguments.\n");
        args->mTargetProcessNames.clear();
    }
    if (args->mTargetPid != 0) {
        PrintWarning("warning: -captureall elides all previous -process_id arguments.\n");
        args->mTargetPid = 0;
    }
}

// Allow /ARG, -ARG, or --ARG
bool ParseArgPrefix(char** arg)
{
    if (**arg == '/') {
        *arg += 1;
        return true;
    }

    if (**arg == '-') {
        *arg += 1;
        if (**arg == '-') {
            *arg += 1;
        }
        return true;
    }

    return false;
}

bool ParseArg(char* arg, char const* option)
{
    return
        ParseArgPrefix(&arg) &&
        _stricmp(arg, option) == 0;
}

bool ParseValue(char** argv, int argc, int* i)
{
    if (*i + 1 < argc) {
        *i += 1;
        return true;
    }
    PrintError("error: %s expecting argument.\n", argv[*i]);
    return false;
}

bool ParseValue(char** argv, int argc, int* i, char const** value)
{
    if (!ParseValue(argv, argc, i)) return false;
    *value = argv[*i];
    return true;
}

bool ParseValue(char** argv, int argc, int* i, std::vector<char const*>* value)
{
    char const* v = nullptr;
    if (!ParseValue(argv, argc, i, &v)) return false;
    value->emplace_back(v);
    return true;
}

bool ParseValue(char** argv, int argc, int* i, UINT* value)
{
    char const* v = nullptr;
    if (!ParseValue(argv, argc, i, &v)) return false;
    *value = strtoul(v, nullptr, 10);
    return true;
}

void PrintHelp()
{
    fprintf(stderr, "PresentMon %s\n", PRESENT_MON_VERSION);

    // Layout usage 
    char* s[] = {
        #include <generated/command_line_options.inl>
    };
    size_t argWidth = 0;
    for (size_t i = 0; i < _countof(s); i += 2) {
        auto arg = s[i];
        auto desc = s[i + 1];
        if (desc != nullptr) {
            argWidth = std::max(argWidth, strlen(arg));
        }
    }

    argWidth = std::min<size_t>(argWidth, MAX_ARG_COLUMN_WIDTH);

    size_t descWidth = std::max<size_t>(MIN_DESC_COLUMN_WIDTH, GetConsoleWidth() - ARG_DESC_COLUMN_PADDING - argWidth);

    // Print usage
    for (size_t i = 0; i < _countof(s); i += 2) {
        auto arg = s[i];
        auto desc = s[i + 1];
        if (desc == nullptr) {
            fprintf(stderr, "\n%s:\n", arg);
        } else {
            fprintf(stderr, "  %-*s  ", (int) argWidth, arg);
            for (auto len = strlen(desc); len > 0; ) {
                if (len <= descWidth) {
                    fprintf(stderr, "%s\n", desc);
                    break;
                }

                auto w = descWidth;
                while (desc[w] != ' ') {
                    --w;
                }
                fprintf(stderr, "%.*s\n%-*s", (int) w, desc, (int) (argWidth + 4), "");
                desc += w + 1;
                len -= w + 1;
            }
        }
    }
}

}

CommandLineArgs const& GetCommandLineArgs()
{
    return gCommandLineArgs;
}

bool ParseCommandLine(int argc, char** argv)
{
    auto args = &gCommandLineArgs;

    args->mTargetProcessNames.clear();
    args->mExcludeProcessNames.clear();
    args->mOutputCsvFileName = nullptr;
    args->mEtlFileName = nullptr;
    args->mSessionName = "PresentMon";
    args->mTargetPid = 0;
    args->mDelay = 0;
    args->mTimer = 0;
    args->mHotkeyModifiers = MOD_NOREPEAT;
    args->mHotkeyVirtualKeyCode = 0;
    args->mTrackDisplay = true;
    args->mTrackDebug = false;
    args->mTrackWMR = false;
    args->mOutputCsvToFile = true;
    args->mOutputCsvToStdout = false;
    args->mOutputQpcTime = false;
    args->mOutputQpcTimeInSeconds = false;
    args->mScrollLockIndicator = false;
    args->mExcludeDropped = false;
    args->mConsoleOutputType = ConsoleOutput::Full;
    args->mTerminateExisting = false;
    args->mTerminateOnProcExit = false;
    args->mStartTimer = false;
    args->mTerminateAfterTimer = false;
    args->mHotkeySupport = false;
    args->mTryToElevate = false;
    args->mMultiCsv = false;
    args->mStopExistingSession = false;

    bool DEPRECATED_dontRestart = false;
    bool DEPRECATED_simple = false;
    bool DEPRECATED_verbose = false;
    bool DEPRECATED_wmr = false;
    for (int i = 1; i < argc; ++i) {
        // Capture target options:
             if (ParseArg(argv[i], "captureall"))   { SetCaptureAll(args);                                         continue; }
        else if (ParseArg(argv[i], "process_name")) { if (ParseValue(argv, argc, &i, &args->mTargetProcessNames))  continue; }
        else if (ParseArg(argv[i], "exclude"))      { if (ParseValue(argv, argc, &i, &args->mExcludeProcessNames)) continue; }
        else if (ParseArg(argv[i], "process_id"))   { if (ParseValue(argv, argc, &i, &args->mTargetPid))           continue; }
        else if (ParseArg(argv[i], "etl_file"))     { if (ParseValue(argv, argc, &i, &args->mEtlFileName))         continue; }

        // Output options:
        else if (ParseArg(argv[i], "output_file"))   { if (ParseValue(argv, argc, &i, &args->mOutputCsvFileName)) continue; }
        else if (ParseArg(argv[i], "output_stdout")) { args->mOutputCsvToStdout      = true;                  continue; }
        else if (ParseArg(argv[i], "multi_csv"))     { args->mMultiCsv               = true;                  continue; }
        else if (ParseArg(argv[i], "no_csv"))        { args->mOutputCsvToFile        = false;                 continue; }
        else if (ParseArg(argv[i], "no_top"))        { args->mConsoleOutputType      = ConsoleOutput::Simple; continue; }
        else if (ParseArg(argv[i], "qpc_time"))      { args->mOutputQpcTime          = true;                  continue; }
        else if (ParseArg(argv[i], "qpc_time_s"))    { args->mOutputQpcTimeInSeconds = true;                  continue; }

        // Recording options:
        else if (ParseArg(argv[i], "hotkey"))           { if (ParseValue(argv, argc, &i) && AssignHotkey(argv[i], args)) continue; }
        else if (ParseArg(argv[i], "delay"))            { if (ParseValue(argv, argc, &i, &args->mDelay)) continue; }
        else if (ParseArg(argv[i], "timed"))            { if (ParseValue(argv, argc, &i, &args->mTimer)) { args->mStartTimer = true; continue; } }
        else if (ParseArg(argv[i], "exclude_dropped"))  { args->mExcludeDropped      = true; continue; }
        else if (ParseArg(argv[i], "scroll_indicator")) { args->mScrollLockIndicator = true; continue; }
        else if (ParseArg(argv[i], "no_track_display")) { args->mTrackDisplay        = false; continue; }
        else if (ParseArg(argv[i], "track_debug"))      { args->mTrackDebug          = true; continue; }
        else if (ParseArg(argv[i], "simple"))           { DEPRECATED_simple          = true; continue; }
        else if (ParseArg(argv[i], "verbose"))          { DEPRECATED_verbose         = true; continue; }

        // Execution options:
        else if (ParseArg(argv[i], "session_name"))           { if (ParseValue(argv, argc, &i, &args->mSessionName)) continue; }
        else if (ParseArg(argv[i], "stop_existing_session"))  { args->mStopExistingSession = true; continue; }
        else if (ParseArg(argv[i], "terminate_existing"))     { args->mTerminateExisting   = true; continue; }
        else if (ParseArg(argv[i], "dont_restart_as_admin"))  { DEPRECATED_dontRestart     = true; continue; }
        else if (ParseArg(argv[i], "restart_as_admin"))       { args->mTryToElevate        = true; continue; }
        else if (ParseArg(argv[i], "terminate_on_proc_exit")) { args->mTerminateOnProcExit = true; continue; }
        else if (ParseArg(argv[i], "terminate_after_timed"))  { args->mTerminateAfterTimer = true; continue; }

        // Beta options:
        else if (ParseArg(argv[i], "track_mixed_reality"))   { args->mTrackWMR = true; continue; }
        else if (ParseArg(argv[i], "include_mixed_reality")) { DEPRECATED_wmr  = true; continue; }

        // Provided argument wasn't recognized
        else if (!(ParseArg(argv[i], "?") || ParseArg(argv[i], "h") || ParseArg(argv[i], "help"))) {
            PrintError("error: unrecognized argument '%s'.\n", argv[i]);
        }

        PrintHelp();
        return false;
    }

    // Handle deprecated command line arguments
    if (DEPRECATED_simple) {
        PrintWarning("warning: -simple command line argument has been deprecated; using -no_track_display instead.\n");
        args->mTrackDisplay = false;
    }
    if (DEPRECATED_verbose) {
        PrintWarning("warning: -verbose command line argument has been deprecated; using -track_debug instead.\n");
        args->mTrackDebug = true;
    }
    if (DEPRECATED_wmr) {
        PrintWarning("warning: -include_mixed_reality command line argument has been deprecated; using -track_mixed_reality instead.\n");
        args->mTrackWMR = true;
    }
    if (DEPRECATED_dontRestart) {
        PrintWarning("warning: -dont_restart_as_admin command line argument has been deprecated; it is now the default behaviour.\n");
    }

    // Ignore -no_track_display if required for other requested tracking
    if (args->mTrackDebug && !args->mTrackDisplay) {
        PrintWarning("warning: -track_debug requires display tracking; ignoring -no_track_display.\n");
        args->mTrackDisplay = true;
    }

    // Enable -qpc_time if only -qpc_time_s was provided, since we use that to
    // add the column.
    if (args->mOutputQpcTimeInSeconds) {
        args->mOutputQpcTime = true;
    }

    // Disallow hotkey of CTRL+C, CTRL+SCROLL, and F12
    if (args->mHotkeySupport) {
        if ((args->mHotkeyModifiers & MOD_CONTROL) != 0 && (
            args->mHotkeyVirtualKeyCode == 0x44 /*C*/ ||
            args->mHotkeyVirtualKeyCode == VK_SCROLL)) {
            PrintError("error: CTRL+C or CTRL+SCROLL cannot be used as a -hotkey, they are reserved for terminating the trace.\n");
            PrintHelp();
            return false;
        }

        if (args->mHotkeyModifiers == MOD_NOREPEAT && args->mHotkeyVirtualKeyCode == VK_F12) {
            PrintError("error: 'F12' cannot be used as a -hotkey, it is reserved for the debugger.\n");
            PrintHelp();
            return false;
        }
    }

    // If -no_csv is used, ignore -qpc_time, -qpc_time_s, -multi_csv,
    // -output_file, or -output_stdout if they are also used.
    if (!args->mOutputCsvToFile) {
        if (args->mOutputQpcTime) {
            PrintWarning("warning: -qpc_time and -qpc_time_s are only relevant for CSV output; ignoring due to -no_csv.\n");
            args->mOutputQpcTime = false;
            args->mOutputQpcTimeInSeconds = false;
        }
        if (args->mMultiCsv) {
            PrintWarning("warning: -multi_csv and -no_csv arguments are not compatible; ignoring -multi_csv.\n");
            args->mMultiCsv = false;
        }
        if (args->mOutputCsvFileName != nullptr) {
            PrintWarning("warning: -output_file and -no_csv arguments are not compatible; ignoring -output_file.\n");
            args->mOutputCsvFileName = nullptr;
        }
        if (args->mOutputCsvToStdout) {
            PrintWarning("warning: -output_stdout and -no_csv arguments are not compatible; ignoring -output_stdout.\n");
            args->mOutputCsvToStdout = false;
        }
    }

    // If we're outputing CSV to stdout, we can't use it for console output.
    //
    // Further, we're currently limited to outputing CSV to either file(s) or
    // stdout, so disallow use of both -output_file and -output_stdout.  Also,
    // since -output_stdout redirects all CSV output to stdout ignore
    // -multi_csv or -track_mixed_reality in this case.
    if (args->mOutputCsvToStdout) {
        args->mConsoleOutputType = ConsoleOutput::None; // No warning needed if user used -no_top, just swap out Simple for None

        if (args->mOutputCsvFileName != nullptr) {
            PrintError("error: only one of -output_file or -output_stdout arguments can be used.\n");
            PrintHelp();
            return false;
        }

        if (args->mMultiCsv) {
            PrintWarning("warning: -multi_csv and -output_stdout are not compatible; ignoring -multi_csv.\n");
            args->mMultiCsv = false;
        }

        if (args->mTrackWMR) {
            PrintWarning("warning: -track_mixed_reality and -output_stdout are not compatible; ignoring -track_mixed_reality.\n");
            args->mTrackWMR = false;
        }
    }

    // Try to initialize the console, and warn if we're not going to be able to
    // do the advanced display as requested.
    if (args->mConsoleOutputType == ConsoleOutput::Full && !args->mOutputCsvToStdout && !IsConsoleInitialized()) {
        if (args->mOutputCsvToFile) {
            PrintWarning("warning: could not initialize console display; continuing with -no_top.\n");
            args->mConsoleOutputType = ConsoleOutput::Simple;
        } else {
            PrintError("error: could not initialize console display; use -no_top or -output_stdout in this environment.\n");
            PrintHelp();
            return false;
        }
    }

    // If -terminate_existing, warn about any normal arguments since we'll just
    // be stopping an existing session and then exiting.
    if (args->mTerminateExisting && (
        !args->mTargetProcessNames.empty() ||
        !args->mExcludeProcessNames.empty() ||
        args->mTargetPid != 0 ||
        args->mEtlFileName != nullptr ||
        args->mOutputCsvFileName != nullptr ||
        args->mOutputCsvToStdout ||
        args->mMultiCsv ||
        args->mOutputCsvToFile == false ||
        args->mConsoleOutputType == ConsoleOutput::Simple ||
        args->mOutputQpcTime ||
        args->mOutputQpcTimeInSeconds ||
        args->mHotkeySupport ||
        args->mDelay != 0 ||
        args->mTimer != 0 ||
        args->mStartTimer ||
        args->mExcludeDropped ||
        args->mScrollLockIndicator ||
        !args->mTrackDisplay ||
        args->mTrackDebug ||
        args->mTrackWMR ||
        args->mTerminateOnProcExit ||
        args->mTerminateAfterTimer)) {
        PrintWarning("warning: -terminate_existing exits without capturing anything; ignoring all capture,\n"
                     "         output, and recording arguments.\n");
    }

    return true;
}
