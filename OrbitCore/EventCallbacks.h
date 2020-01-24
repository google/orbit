//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <evntrace.h>
#include "Core.h"

namespace EventTracing {
typedef void (*EventCallback)(PEVENT_RECORD a_EventRecord, UCHAR a_Opcode);

void Callback(PEVENT_RECORD a_EventRecord);
void CallbackALPC(PEVENT_RECORD a_EventRecord, UCHAR a_Opcode);
void CallbackDiskIo(PEVENT_RECORD a_EventRecord, UCHAR a_Opcode);
void CallbackEventTraceConfig(PEVENT_RECORD a_EventRecord, UCHAR a_Opcode);
void CallbackFileIo(PEVENT_RECORD a_EventRecord, UCHAR a_Opcode);
void CallbackImageLoad(PEVENT_RECORD a_EventRecord, UCHAR a_Opcode);
void CallbackPageFault(PEVENT_RECORD a_EventRecord, UCHAR a_Opcode);
void CallbackPerfInfo(PEVENT_RECORD a_EventRecord, UCHAR a_Opcode);
void CallbackProcess(PEVENT_RECORD a_EventRecord, UCHAR a_Opcode);
void CallbackRegistry(PEVENT_RECORD a_EventRecord, UCHAR a_Opcode);
void CallbackSplitIo(PEVENT_RECORD a_EventRecord, UCHAR a_Opcode);
void CallbackTcpIp(PEVENT_RECORD a_EventRecord, UCHAR a_Opcode);
void CallbackThread(PEVENT_RECORD a_EventRecord, UCHAR a_Opcode);
void CallbackUdpIp(PEVENT_RECORD a_EventRecord, UCHAR a_Opcode);
void CallbackStackWalk(PEVENT_RECORD a_EventRecord, UCHAR a_Opcode);
void Init();
void Reset();
}  // namespace EventTracing