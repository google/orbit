// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_EVENT_TYPES_H_
#define WINDOWS_TRACING_EVENT_TYPES_H_

#include <cstdint>

namespace orbit_windows_tracing::etw {

// ETW event types grouped by event class. Note that an event type name is of the form
// "[parent class]_[event class]". The parent class is associated with a guid that is used
// to dispatch events to a particular callback for processing.
// See https://docs.microsoft.com/en-us/windows/win32/etw/event-tracing-mof-classes.

// https://docs.microsoft.com/en-us/windows/win32/etw/thread-typegroup1
namespace Thread_TypeGroup1 {
static constexpr uint8_t kStart = 1;
static constexpr uint8_t kEnd = 2;
static constexpr uint8_t kDcStart = 3;
static constexpr uint8_t kDcEnd = 4;
};  // namespace Thread_TypeGroup1

// https://docs.microsoft.com/en-us/windows/win32/etw/cswitch
namespace Thread_CSwitch {
static constexpr uint8_t kCSwitch = 36;
};

// https://docs.microsoft.com/en-us/windows/win32/etw/sampledprofile
namespace PerfInfo_SampledProfile {
static constexpr uint8_t kSampleProfile = 46;
};

// https://docs.microsoft.com/en-us/windows/win32/etw/diskio-typegroup1
namespace DiskIo_TypeGroup1 {
static constexpr uint8_t kRead = 10;
static constexpr uint8_t kWrite = 11;
};  // namespace DiskIo_TypeGroup1

// https://docs.microsoft.com/en-us/windows/win32/etw/diskio-typegroup2
namespace DiskIo_TypeGroup2 {
static constexpr uint8_t kReadInit = 12;
static constexpr uint8_t kWriteInit = 13;
static constexpr uint8_t kFlushInit = 15;
};  // namespace DiskIo_TypeGroup2

// https://docs.microsoft.com/is-is/windows/win32/etw/diskio-typegroup3
namespace DiskIo_TypeGroup3 {
static constexpr uint8_t kFlushBuffer = 14;
};

// https://docs.microsoft.com/en-us/windows/win32/etw/fileio-name
namespace FileIo_Name {
static constexpr uint8_t kName = 0;
static constexpr uint8_t kFileCreate = 32;
static constexpr uint8_t kFileDelete = 35;
static constexpr uint8_t kFileRunDown = 36;
};  // namespace FileIo_Name

// https://docs.microsoft.com/en-us/windows/win32/etw/fileio-create
namespace FileIo_Create {
static constexpr uint8_t kFileCreate = 64;
};

// https://docs.microsoft.com/en-us/windows/win32/etw/fileio-info
namespace FileIo_Info {
static constexpr uint8_t kSetInfo = 69;
static constexpr uint8_t kDelete = 70;
static constexpr uint8_t kRename = 71;
static constexpr uint8_t kQueryInfo = 74;
static constexpr uint8_t kFsControl = 75;
};  // namespace FileIo_Info

// https://docs.microsoft.com/en-us/windows/win32/etw/fileio-readwrite
namespace FileIo_ReadWrite {
static constexpr uint8_t kRead = 67;
static constexpr uint8_t kWrite = 68;
};  // namespace FileIo_ReadWrite

// https://docs.microsoft.com/en-us/windows/win32/etw/stackwalk-event
namespace StackWalk_Event {
static constexpr uint8_t kStack = 32;
};

}  // namespace orbit_windows_tracing::etw

#endif  // WINDOWS_TRACING_EVENT_TYPES_H_
