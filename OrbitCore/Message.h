//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "BaseTypes.h"
#include <string>

#ifndef _WINDEF_
typedef unsigned long DWORD;
#endif

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
    Msg_OrbitData
};

//-----------------------------------------------------------------------------
struct MessageGeneric
{
    ULONG64 m_Address;
};

//-----------------------------------------------------------------------------
struct DataTransferHeader
{
    enum DataType{ Data, Code };
    ULONG64  m_Address;
    DataType m_Type;
};

//-----------------------------------------------------------------------------
struct ArgTrackingHeader
{
    ULONG64 m_Function;
    int     m_NumArgs;
};

//-----------------------------------------------------------------------------
struct UnrealObjectHeader
{
    ULONG64 m_Ptr;
    int     m_StrSize : 31;
    bool    m_WideStr  : 1;
};

//-----------------------------------------------------------------------------
#pragma pack(push, 1)
class Message
{
public:
    Message ( MessageType a_Type = Msg_Invalid
            , int a_Size = 0
            , char* a_Data = nullptr )
            : m_Type(a_Type)
            , m_Size(a_Size)
            , m_SessionID(GSessionID)
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

public:
    MessageType    m_Type;
    Header         m_Header;
    int            m_Size;
    int            m_SessionID;
    char*          m_Data;
#ifndef _WIN64
    char*          m_Padding; // Make sure Message is same size on both Win32 and x64
#endif

    static int GSessionID;
};

//-----------------------------------------------------------------------------
struct OrbitZoneName
{
    enum { NUM_CHAR = 64 };
    DWORD64 m_Address;
    char    m_Data[NUM_CHAR];
};

//-----------------------------------------------------------------------------
struct OrbitLogEntry
{
    OrbitLogEntry() : m_Time( 0 ), m_CallstackHash( 0 ), m_ThreadId( 0 ) {}
    DWORD64     m_Time;
    DWORD64     m_CallstackHash;
    DWORD       m_ThreadId;
    std::string m_Text; // this must be the last member

    static size_t GetSizeWithoutString() { return sizeof(OrbitLogEntry) - sizeof(std::string); }
    size_t GetStringSize() { return ((m_Text.size()+1)*sizeof(m_Text[0])); }
    size_t GetBufferSize() { return GetSizeWithoutString() + GetStringSize(); }
};

//-----------------------------------------------------------------------------
struct OrbitWaitLoop
{
    DWORD         m_ThreadId;
    DWORD64       m_Address;
    unsigned char m_OriginalBytes[2];
};

//-----------------------------------------------------------------------------
struct OrbitUnrealInfo
{
    OrbitUnrealInfo(){ memset(this, 0, sizeof(*this) ); }
    DWORD64       m_GetDisplayNameEntryAddress;
    DWORD         m_UobjectNameOffset;
    DWORD         m_EntryNameOffset;
    DWORD         m_EntryIndexOffset;
};

#pragma pack(pop)

