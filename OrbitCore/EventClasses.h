//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Core.h"

//-----------------------------------------------------------------------------
//[EventType{ 1, 2, 3, 4 }, EventTypeName{ "Start", "End", "DCStart", "DCEnd" }]
struct Thread_TypeGroup1
{
    uint32_t ProcessId;
    uint32_t TThreadId;
    ptr_type StackBase;
    ptr_type StackLimit;
    ptr_type UserStackBase;
    ptr_type UserStackLimit;
    ptr_type Affinity;
    ptr_type Win32StartAddr;
    ptr_type TebBase;
    uint32_t SubProcessTag;
    uint8_t  BasePriority;
    uint8_t  PagePriority;
    uint8_t  IoPriority;
    uint8_t  ThreadFlags;

    static const UCHAR OPCODE_START = 1;
    static const UCHAR OPCODE_END = 2;
    static const UCHAR OPCODE_DC_START = 3;
    static const UCHAR OPCODE_DC_END = 4;
};

//-----------------------------------------------------------------------------
// [EventType{36}, EventTypeName{"CSwitch"}]
struct CSwitch
{
    uint32_t NewThreadId;
    uint32_t OldThreadId;
    int8_t   NewThreadPriority;
    int8_t   OldThreadPriority;
    uint8_t  PreviousCState;
    int8_t   SpareByte;
    int8_t   OldThreadWaitReason;
    int8_t   OldThreadWaitMode;
    int8_t   OldThreadState;
    int8_t   OldThreadWaitIdealProcessor;
    uint32_t NewThreadWaitTime;
    uint32_t Reserved;

    static const UCHAR OPCODE = 36;
};

//-----------------------------------------------------------------------------
//[EventType{ 46 }, EventTypeName{ "SampleProfile" }]
struct PerfInfo_SampledProfile
{
    ptr_type InstructionPointer;
    uint32_t ThreadId;
    uint32_t Count;

    static const UCHAR OPCODE = 46;
};

//-----------------------------------------------------------------------------
//[EventType{ 10,11 }, EventTypeName{ "Read","Write" }]
struct DiskIo_TypeGroup1
{
    uint32_t DiskNumber;
    uint32_t IrpFlags;
    uint32_t TransferSize;
    uint32_t Reserved;
    int64_t  ByteOffset;
    ptr_type FileObject;
    ptr_type Irp;
    uint64_t HighResResponseTime;
    uint32_t IssuingThreadId;

    static const UCHAR OPCODE_READ  = EVENT_TRACE_TYPE_IO_READ;
    static const UCHAR OPCODE_WRITE = EVENT_TRACE_TYPE_IO_WRITE;
};

//-----------------------------------------------------------------------------
//[EventType{ 12, 13, 15 }, EventTypeName{ "ReadInit", "WriteInit", "FlushInit" }]
struct DiskIo_TypeGroup2
{
    ptr_type Irp;
    uint32_t IssuingThreadId;

    static const UCHAR OPCODE_READ_INIT = EVENT_TRACE_TYPE_IO_READ_INIT;
    static const UCHAR OPCODE_WRITE_INIT = EVENT_TRACE_TYPE_IO_WRITE_INIT;
    static const UCHAR OPCODE_FLUSH_INIT = EVENT_TRACE_TYPE_IO_FLUSH_INIT;
};

//-----------------------------------------------------------------------------
//[EventType{ 14 }, EventTypeName{ "FlushBuffers" }]
struct DiskIo_TypeGroup3
{
    uint32_t DiskNumber;
    uint32_t IrpFlags;
    uint64_t HighResResponseTime;
    ptr_type Irp;
    uint32_t IssuingThreadId;

    static const UCHAR OPCODE_FLUSH_BUFFER = EVENT_TRACE_TYPE_IO_FLUSH;
};

//-----------------------------------------------------------------------------
//[EventType{ 0, 32, 35, 36 }, EventTypeName{ "Name", "FileCreate", "FileDelete", "FileRundown" }]
struct FileIo_Name
{
    ptr_type FileObject;
    wchar_t  FileName[1];

    static const UCHAR OPCODE_NAME = 0;
    static const UCHAR OPCODE_FILE_CREATE = 32;
    static const UCHAR OPCODE_FILE_DELETE = 35;
    static const UCHAR OPCODE_FILE_RUNDOWN = 36;
};

//-----------------------------------------------------------------------------
//[EventType{ 64 }, EventTypeName{ "Create" }]
struct FileIo_Create
{
    ptr_type IrpPtr;
    ptr_type TTID;
    ptr_type FileObject;
    uint32_t CreateOptions;
    uint32_t FileAttributes;
    uint32_t ShareAccess;
    wchar_t  OpenPath[1];

    static const UCHAR OPCODE_FILE_CREATE = 64;
};

//-----------------------------------------------------------------------------
//[EventType{ 69, 70, 71, 74, 75 }, EventTypeName{ "SetInfo", "Delete", "Rename", "QueryInfo", "FSControl" }]
struct FileIo_Info
{
    ptr_type IrpPtr;
    ptr_type TTID;
    ptr_type FileObject;
    ptr_type FileKey;
    ptr_type ExtraInfo;
    uint32_t InfoClass;

    static const UCHAR OPCODE_SET_INFO = 69;
    static const UCHAR OPCODE_DELETE = 70;
    static const UCHAR OPCODE_RENAME = 71;
    static const UCHAR OPCODE_QUERY_INFO = 74;
    static const UCHAR OPCODE_FS_CONTROL = 75;
};

//-----------------------------------------------------------------------------
//[EventType{ 67, 68 }, EventTypeName{ "Read", "Write" }]
struct FileIo_ReadWrite
{
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa964772(v=vs.85).aspx

    uint64_t Offset;
    ptr_type IrpPtr;
    ptr_type TTID;
    ptr_type FileObject;
    ptr_type FileKey; //To determine the file name, match the value of this property to the FileObject property of a FileIo_Name event.
    uint32_t IoSize;
    uint32_t IoFlags;

    static const UCHAR OPCODE_READ = 67;
    static const UCHAR OPCODE_WRITE = 68;
};

// 0	// File name event.The FileIo_Name MOF class defines the event data for this event.
// 32	// File create event.The FileIo_Name MOF class defines the event data for this event.
// 35	// File delete event.The FileIo_Name MOF class defines the event data for this event.
// 36	// File rundown event.Enumerates all open files on the computer at the end of the trace session.The FileIo_Name MOF class defines the event data for this event.
// 64	// File create event.The FileIo_Create MOF class defines the event data for this event.
// 72	// Directory enumeration event.The FileIo_DirEnum MOF class defines the event data for this event.
// 77	// Directory notification event.The FileIo_DirEnum MOF class defines the event data for this event.
// 69	// Set information event.The FileIo_Info MOF class defines the event data for this event.
// 70	// Delete file event.The FileIo_Info MOF class defines the event data for this event.
// 71	// Rename file event.The FileIo_Info MOF class defines the event data for this event.
// 74	// Query file information event.The FileIo_Info MOF class defines the event data for this event.
// 75	// File system control event.The FileIo_Info MOF class defines the event data for this event.
// 76	// End of operation event.The FileIo_OpEnd MOF class defines the event data for this event.
// 67	// File read event.The FileIo_ReadWrite MOF class defines the event data for this event.
// 68	// File write event.The FileIo_ReadWrite MOF class defines the event data for this event.
// 65	// Clean up event.The event is generated when the last handle to the file is released.The FileIo_SimpleOp MOF class defines the event data for this event.
// 66	// Close event.The event is generated when the file object is freed.The FileIo_SimpleOp MOF class defines the event data for this event.
// 73	// Flush event.This event is generated when the file buffers are fully flushed to disk.The FileIo_SimpleOp MOF class defines the event data for this event.

//-----------------------------------------------------------------------------
typedef struct _STACK_TRACING_EVENT_ID {
    GUID  EventGuid;
    UCHAR Type;
    UCHAR Reserved[7];
} STACK_TRACING_EVENT_ID, *PSTACK_TRACING_EVENT_ID;

//-----------------------------------------------------------------------------
//[EventType{ 32 }, EventTypeName{ "Stack" }]
struct StackWalk_Event
{
    uint64_t EventTimeStamp;
    uint32_t StackProcess;
    uint32_t StackThread;
    ptr_type Stack1;

    static const UCHAR OPCODE_STACK = 32;
};