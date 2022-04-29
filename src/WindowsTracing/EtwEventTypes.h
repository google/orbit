// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_ETW_EVENT_TYPES_H_
#define WINDOWS_TRACING_ETW_EVENT_TYPES_H_

#include <cstdint>

namespace orbit_windows_tracing {

// ETW event types grouped by event class. See Microsoft's documentation for more info:
// https://docs.microsoft.com/en-us/windows/win32/etw/event-tracing-mof-classes.

// https://docs.microsoft.com/en-us/windows/win32/etw/thread-typegroup1
static constexpr uint8_t kEtwThreadGroup1EventStart = 1;
static constexpr uint8_t kEtwThreadGroup1EventEnd = 2;
static constexpr uint8_t kEtwThreadGroup1EventDcStart = 3;
static constexpr uint8_t kEtwThreadGroup1EventDcEnd = 4;

// https://docs.microsoft.com/en-us/windows/win32/etw/cswitch
static constexpr uint8_t kEtwThreadV2EventCSwitch = 36;

// https://docs.microsoft.com/en-us/windows/win32/etw/sampledprofile
static constexpr uint8_t kSampledProfileEventSampleProfile = 46;

// https://docs.microsoft.com/en-us/windows/win32/etw/diskio-typegroup1
static constexpr uint8_t kEtwDiskIoGroup1EventRead = 10;
static constexpr uint8_t kEtwDiskIoGroup1EventWrite = 11;

// https://docs.microsoft.com/en-us/windows/win32/etw/diskio-typegroup2
static constexpr uint8_t kEtwDiskIoGroup2EventReadInit = 12;
static constexpr uint8_t kEtwDiskIoGroup2EventWriteInit = 13;
static constexpr uint8_t kEtwDiskIoGroup2EventFlushInit = 15;

// https://docs.microsoft.com/is-is/windows/win32/etw/diskio-typegroup3
static constexpr uint8_t kEtwDiskIoGroup3EventFlushBuffer = 14;

// https://docs.microsoft.com/en-us/windows/win32/etw/fileio-name
static constexpr uint8_t kEtwFileIoNameEventName = 0;
static constexpr uint8_t kEtwFileIoNameEventFileCreate = 32;
static constexpr uint8_t kEtwFileIoNameEventFileDelete = 35;
static constexpr uint8_t kEtwFileIoNameEventFileRunDown = 36;

// https://docs.microsoft.com/en-us/windows/win32/etw/fileio-create
static constexpr uint8_t kEtwFileIoCreateEventCreate = 64;

// https://docs.microsoft.com/en-us/windows/win32/etw/fileio-info
static constexpr uint8_t kEtwFileIoInfoEventSetInfo = 69;
static constexpr uint8_t kEtwFileIoInfoEventDelete = 70;
static constexpr uint8_t kEtwFileIoInfoEventRename = 71;
static constexpr uint8_t kEtwFileIoInfoEventQueryInfo = 74;
static constexpr uint8_t kEtwFileIoInfoEventFsControl = 75;

// https://docs.microsoft.com/en-us/windows/win32/etw/fileio-readwrite
static constexpr uint8_t kEtwFileIoReadWriteEventRead = 67;
static constexpr uint8_t kEtwFileIoReadWriteEventWrite = 68;

// https://docs.microsoft.com/en-us/windows/win32/etw/stackwalk-event
static constexpr uint8_t kStackWalkEventStack = 32;

// https://docs.microsoft.com/is-is/windows/win32/etw/image-load
static constexpr uint8_t kImageLoadEventLoad = 10;
static constexpr uint8_t kImageLoadEventUnload = 2;
static constexpr uint8_t kImageLoadEventDcStart = 3;
static constexpr uint8_t kImageLoadEventDcEnd = 4;

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_EVENT_TYPES_H_
