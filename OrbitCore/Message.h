// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <string>
#include <vector>

#include "BaseTypes.h"

#pragma pack(push, 1)

//-----------------------------------------------------------------------------
enum MessageType : int16_t {
  Msg_Invalid,
  Msg_SetData,
  Msg_GetData,
  Msg_String,
  Msg_Timer,
  Msg_NewCaptureID,
  Msg_StartCapture,
  Msg_StopCapture,
  Msg_FunctionHook,
  Msg_SavedContext,
  Msg_ClearArgTracking,
  Msg_ArgTracking,
  Msg_CallstackTracking,
  Msg_Unload,
  Msg_NumQueuedEntries,
  Msg_NumFlushedEntries,
  Msg_NumFlushedItems,
  Msg_NumInstalledHooks,
  Msg_Callstack,
  Msg_OrbitZoneName,
  Msg_OrbitLog,
  Msg_WaitLoop,
  Msg_ThawMainThread,
  Msg_OrbitUnrealInfo,
  Msg_OrbitUnrealObject,
  Msg_MiniDump,
  Msg_UserData,
  Msg_OrbitData,
  Msg_ThreadInfo,
  Msg_CrossPlatform,
  Msg_RemoteModule,
  Msg_RemoteFunctions,
  Msg_RemoteModuleDebugInfo,
  Msg_Timers,
  Msg_RemoteCallStack,
  Msg_LinuxAddressInfos,
  Msg_SamplingCallstack,
  Msg_TimerCallstack,
  Msg_RemoteSelectedFunctionsMap,
  Msg_SamplingCallstacks,
  Msg_SamplingHashedCallstacks,
  Msg_KeysAndStrings,
  Msg_ThreadNames,
  Msg_ValidateFramePointers,
  Msg_CaptureStopped
};

//-----------------------------------------------------------------------------
struct MessageGeneric {
  uint64_t m_Address;
};

//-----------------------------------------------------------------------------
struct DataTransferHeader {
  enum DataType { Data, Code };
  uint64_t m_Address;
  DataType m_Type;
};

//-----------------------------------------------------------------------------
struct ArgTrackingHeader {
  uint64_t m_Function;
  uint32_t m_NumArgs;
};

//-----------------------------------------------------------------------------
struct UnrealObjectHeader {
  uint64_t m_Ptr;
  uint32_t m_StrSize;
  bool m_WideStr;
};

//-----------------------------------------------------------------------------
class Message {
 public:
  Message(MessageType a_Type = Msg_Invalid, uint32_t a_Size = 0,
          char* a_Data = nullptr)
      : m_Type(a_Type),
        m_Size(a_Size),
        m_CaptureID(GCaptureID),
        m_ThreadId(0),
        m_Data(a_Data) {}

 public:
  union Header {
    MessageGeneric m_GenericHeader;
    DataTransferHeader m_DataTransferHeader;
    ArgTrackingHeader m_ArgTrackingHeader;
    UnrealObjectHeader m_UnrealObjectHeader;
  };

  MessageType GetType() const { return m_Type; }
  const Header& GetHeader() const { return m_Header; }
  const void* GetData() const { return m_Data; }
  void* GetData() { return m_Data; }
  uint32_t GetSize() const { return m_Size; }
  std::string GetDataAsString() const {
    return std::string(static_cast<const char*>(m_Data), m_Size);
  }
  static void Dump();

 public:
  MessageType m_Type;
  Header m_Header;
  uint32_t m_Size;
  uint32_t m_CaptureID;
  int32_t m_ThreadId;
  void* m_Data;
#ifdef WIN32
#ifndef _WIN64
  char* m_Padding;  // Make sure Message is same size on both Win32 and x64
#endif
#endif

  static uint32_t GCaptureID;
};

class MessageOwner : public Message {
 public:
  MessageOwner(Message a_Message) {
    Message* message = this;
    *message = a_Message;
    m_OwnedData.resize(m_Size);
    memcpy(m_OwnedData.data(), m_Data, m_Size);
    m_Data = m_OwnedData.data();
  }
  const void* Data() const { return m_OwnedData.data(); }

 private:
  MessageOwner();
  std::vector<char> m_OwnedData;
};

//-----------------------------------------------------------------------------
struct OrbitZoneName {
  enum { NUM_CHAR = 64 };
  uint64_t m_Address;
  char m_Data[NUM_CHAR];
};

//-----------------------------------------------------------------------------
struct OrbitWaitLoop {
  uint32_t m_ThreadId;
  uint64_t m_Address;
  unsigned char m_OriginalBytes[2];
};

//-----------------------------------------------------------------------------
struct OrbitUnrealInfo {
  OrbitUnrealInfo() { memset(this, 0, sizeof(*this)); }
  uint64_t m_GetDisplayNameEntryAddress;
  uint32_t m_UobjectNameOffset;
  uint32_t m_EntryNameOffset;
  uint32_t m_EntryIndexOffset;
};

//-----------------------------------------------------------------------------
struct OrbitLogEntry {
  OrbitLogEntry() : m_Time(0), m_CallstackHash(0), m_ThreadId(0) {}
  uint64_t m_Time;
  uint64_t m_CallstackHash;
  uint32_t m_ThreadId;
  std::string m_Text;  // this must be the last member

  static size_t GetSizeWithoutString() {
    return sizeof(OrbitLogEntry) - sizeof(std::string);
  }
  size_t GetStringSize() { return ((m_Text.size() + 1) * sizeof(m_Text[0])); }
  size_t GetBufferSize() { return GetSizeWithoutString() + GetStringSize(); }
};

#pragma pack(pop)
