//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "BaseTypes.h"
#include <string>
#include <vector>

#pragma pack(push, 1)

//-----------------------------------------------------------------------------
enum MessageType : int16_t
{
    Msg_Invalid,
    Msg_SetData,
    Msg_GetData,
    Msg_String,
    Msg_Timer,
    Msg_NewSession,
    Msg_StartCapture,
    Msg_StopCapture,
    Msg_FunctionHook,
    Msg_FunctionHookZoneStart,
    Msg_FunctionHookZoneStop,
    Msg_FunctionHookOutputDebugString,
    Msg_FunctionHookUnrealActor,
    Msg_FunctionHookAlloc,
    Msg_FunctionHookFree,
    Msg_FunctionHookRealloc,
    Msg_FunctionHookOrbitData,
    Msg_SavedContext,
    Msg_ClearArgTracking,
    Msg_ArgTracking,
    Msg_CallstackTracking,
    Msg_Unload,
    Msg_WebSocketHandshake,
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
    Msg_RemoteProcess,
    Msg_RemoteProcessRequest,
    Msg_RemoteModule,
    Msg_RemoteFunctions,
    Msg_RemotePerf,
    Msg_RemoteProcessList,
    Msg_RemoteModuleDebugInfo,
    Msg_BpfScript,
    Msg_RemoteTimers,
};

//-----------------------------------------------------------------------------
struct MessageGeneric
{
    uint64_t m_Address;
};

//-----------------------------------------------------------------------------
struct DataTransferHeader
{
    enum DataType{ Data, Code };
    uint64_t m_Address;
    DataType m_Type;
};

//-----------------------------------------------------------------------------
struct ArgTrackingHeader
{
    uint64_t m_Function;
    uint32_t m_NumArgs;
};

//-----------------------------------------------------------------------------
struct UnrealObjectHeader
{
    uint64_t m_Ptr;
    uint32_t m_StrSize;
    bool     m_WideStr;
};

//-----------------------------------------------------------------------------
class Message
{
public:
    Message ( MessageType a_Type = Msg_Invalid
            , uint32_t a_Size = 0
            , char* a_Data = nullptr )
            : m_Type(a_Type)
            , m_Size(a_Size)
            , m_SessionID(GSessionID)
            , m_ThreadId(0)
            , m_Data(a_Data)
    {}

public:
    union Header
    {
        MessageGeneric     m_GenericHeader;
        DataTransferHeader m_DataTransferHeader;
        ArgTrackingHeader  m_ArgTrackingHeader;
        UnrealObjectHeader m_UnrealObjectHeader;
    };

    MessageType    GetType()   const { return m_Type; }
    const Header & GetHeader() const { return m_Header; }
    const char*    GetData()   const { return m_Data; }
    char*          GetData()         { return m_Data; }
    static void    Dump();

public:
    MessageType    m_Type;
    Header         m_Header;
    uint32_t       m_Size;
    uint32_t       m_SessionID;
    uint32_t       m_ThreadId;
    char*          m_Data;
#ifdef WIN32
#ifndef _WIN64
    char*          m_Padding; // Make sure Message is same size on both Win32 and x64
#endif
#endif

    static uint32_t GSessionID;
};

class MessageOwner : public Message
{
public:
    MessageOwner(Message a_Message)
    {
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
struct OrbitZoneName
{
    enum { NUM_CHAR = 64 };
    uint64_t m_Address;
    char     m_Data[NUM_CHAR];
};

//-----------------------------------------------------------------------------
struct OrbitWaitLoop
{
    uint32_t      m_ThreadId;
    uint64_t      m_Address;
    unsigned char m_OriginalBytes[2];
};

//-----------------------------------------------------------------------------
struct OrbitUnrealInfo
{
    OrbitUnrealInfo(){ memset(this, 0, sizeof(*this) ); }
    uint64_t m_GetDisplayNameEntryAddress;
    uint32_t m_UobjectNameOffset;
    uint32_t m_EntryNameOffset;
    uint32_t m_EntryIndexOffset;
};

//-----------------------------------------------------------------------------
struct OrbitLogEntry
{
    OrbitLogEntry() : m_Time(0), m_CallstackHash(0), m_ThreadId(0) {}
    uint64_t    m_Time;
    uint64_t    m_CallstackHash;
    uint32_t    m_ThreadId;
    std::string m_Text; // this must be the last member

    static size_t GetSizeWithoutString() { return sizeof(OrbitLogEntry) - sizeof(std::string); }
    size_t GetStringSize() { return ((m_Text.size() + 1) * sizeof(m_Text[0])); }
    size_t GetBufferSize() { return GetSizeWithoutString() + GetStringSize(); }
};

#pragma pack(pop)
