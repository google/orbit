// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_EVENT_TYPES_H_
#define WINDOWS_TRACING_EVENT_TYPES_H_

#include <cstdint>

// https://docs.microsoft.com/en-us/windows/win32/etw/thread-typegroup1
// [EventType{ 1, 2, 3, 4 }, EventTypeName{ "Start", "End", "DCStart", "DCEnd" }]
struct Thread_TypeGroup1 {
  static constexpr uint8_t kOpcodeStart = 1;
  static constexpr uint8_t kOpcodeEnd = 2;
  static constexpr uint8_t kOpcodeDcStart = 3;
  static constexpr uint8_t kOpcodeDcEnd = 4;
};

// https://docs.microsoft.com/en-us/windows/win32/etw/cswitch
// [EventType{36}, EventTypeName{"CSwitch"}]
struct Thread_CSwitch {
  static constexpr uint8_t kOpcodeCSwitch = 36;
};

// https://docs.microsoft.com/en-us/windows/win32/etw/sampledprofile
// [EventType{ 46 }, EventTypeName{ "SampleProfile" }]
struct PerfInfo_SampledProfile {
  static constexpr uint8_t kOpcodeSampleProfile = 46;
};

// https://docs.microsoft.com/en-us/windows/win32/etw/diskio-typegroup1
// [EventType{ 10,11 }, EventTypeName{ "Read","Write" }]
struct DiskIo_TypeGroup1 {
  static constexpr uint8_t kOpcodeRead = 10;
  static constexpr uint8_t kOpcodeWrite = 11;
};

// https://docs.microsoft.com/en-us/windows/win32/etw/diskio-typegroup2
// [EventType{ 12, 13, 15 }, EventTypeName{ "ReadInit", "WriteInit", "FlushInit" }]
struct DiskIo_TypeGroup2 {
  static constexpr uint8_t kOpcodeReadInit = 12;
  static constexpr uint8_t kOpcodeWriteInit = 13;
  static constexpr uint8_t kOpcodeFlushInit = 15;
};

// https://docs.microsoft.com/is-is/windows/win32/etw/diskio-typegroup3
// [EventType{ 14 }, EventTypeName{ "FlushBuffers" }]
struct DiskIo_TypeGroup3 {
  static constexpr uint8_t kOpcodeFlushBuffer = 14;
};

// https://docs.microsoft.com/en-us/windows/win32/etw/fileio-name
// [EventType{ 0, 32, 35, 36 }, EventTypeName{ "Name", "FileCreate", "FileDelete", "FileRundown" }]
struct FileIo_Name {
  static constexpr uint8_t kOpcodeName = 0;
  static constexpr uint8_t kOpcodeFileCreate = 32;
  static constexpr uint8_t kOpcodeFileDelete = 35;
  static constexpr uint8_t kOpcodeFileRunDown = 36;
};

// https://docs.microsoft.com/en-us/windows/win32/etw/fileio-create
// [EventType{ 64 }, EventTypeName{ "Create" }]
struct FileIo_Create {
  static constexpr uint8_t kOpcodeFileCreate = 64;
};

// https://docs.microsoft.com/en-us/windows/win32/etw/fileio-info
// [EventType{ 69, 70, 71, 74, 75 }, EventTypeName{ "SetInfo", "Delete", "Rename", "QueryInfo",
// "FSControl" }]
struct FileIo_Info {
  static constexpr uint8_t kOpcodeSetInfo = 69;
  static constexpr uint8_t kOpcodeDelete = 70;
  static constexpr uint8_t kOpcodeRename = 71;
  static constexpr uint8_t kOpcodeQueryInfo = 74;
  static constexpr uint8_t kOpcodeFsControl = 75;
};

// https://docs.microsoft.com/en-us/windows/win32/etw/fileio-readwrite
// [EventType{ 67, 68 }, EventTypeName{ "Read", "Write" }]
struct FileIo_ReadWrite {
  static constexpr uint8_t kOpcodeRead = 67;
  static constexpr uint8_t kOpcodeWrite = 68;
};

// https://docs.microsoft.com/en-us/windows/win32/etw/stackwalk-event
// [EventType{ 32 }, EventTypeName{ "Stack" }]
struct StackWalk_Event {
  static constexpr uint8_t kOpcodeStack = 32;
};

#endif  // WINDOWS_TRACING_EVENT_TYPES_H_
