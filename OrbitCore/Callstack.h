//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "CallstackTypes.h"
#include "OrbitDbgHelp.h"
#include "PrintVar.h"
#include "ScopeTimer.h"
#include "SerializationMacros.h"

class CallStackPOD {
 public:
  CallStackPOD() { memset(this, 0, sizeof(*this)); }
  static CallStackPOD Walk(uint64_t ip, uint64_t sp);
  static inline CallStackPOD GetCallstackManual(
      uint64_t a_ProgramCounter, uint64_t a_AddressOfReturnAddress);
  size_t GetSizeInBytes() {
    return offsetof(CallStackPOD, data_) + depth_ * sizeof(data_[0]);
  }

  void CalculateHash() {
    hash_ = XXH64(&data_[0], depth_ * sizeof(data_[0]), 0xca1157ac);
  }

  CallstackID Hash() const { return hash_; }

  ThreadID ThreadId() const { return thread_id_; }
  size_t Depth() const { return depth_; }
  const uint64_t* Data() const { return data_; }

 private:
  // Callstack needs to be POD
  CallstackID hash_;
  size_t depth_;
  ThreadID thread_id_;
  uint64_t data_[ORBIT_STACK_SIZE];  // Needs to be last member
};

//-----------------------------------------------------------------------------
struct CallStack {
  CallStack() = default;
  explicit CallStack(CallStackPOD a_CS);
  inline CallstackID Hash() {
    if (m_Hash != 0) return m_Hash;
    m_Hash = XXH64(m_Data.data(), m_Depth * sizeof(uint64_t), 0xca1157ac);
    return m_Hash;
  }
  void Print();
  std::string GetString();
  void Clear() {
    m_Data.clear();
    m_Hash = m_Depth = m_ThreadId = 0;
  }

  CallstackID m_Hash = 0;
  uint32_t m_Depth = 0;
  ThreadID m_ThreadId = 0;
  std::vector<uint64_t> m_Data;

  ORBIT_SERIALIZABLE;
};

//-----------------------------------------------------------------------------
struct HashedCallStack {
  CallstackID m_Hash = 0;
  ThreadID m_ThreadId = 0;

  ORBIT_SERIALIZABLE;
};

#ifdef _WIN32
//-----------------------------------------------------------------------------
struct StackFrame {
  StackFrame(HANDLE a_Thread);
  CONTEXT m_Context;
  STACKFRAME64 m_StackFrame;
  DWORD m_ImageType;
  CallStack m_Callstack;
};

//-----------------------------------------------------------------------------
inline CallStack GetCallstackRtl() {
  // SCOPE_TIMER_LOG( "GetCallstack()" );
  CallStack cs;
  void* stack[ORBIT_STACK_SIZE];
  ULONG hash;
  USHORT numFrames =
      RtlCaptureStackBackTrace(2, ORBIT_STACK_SIZE, stack, &hash);

  for (USHORT i = 0; i < numFrames; ++i) {
    cs.m_Data[i] = (DWORD64)stack[i];
  }

  cs.m_Hash = hash;
  cs.m_Depth = numFrames;
  cs.m_ThreadId = GetCurrentThreadId();

  return cs;
}

#ifndef _WIN64
//-----------------------------------------------------------------------------
inline CallStack GetCallstack(uint64_t a_ProgramCounter,
                              uint64_t a_AddressOfReturnAddress) {
  unsigned int depth = 0;

  SetLastError(0);
  PRINT_VAR(GetLastErrorAsString());

  HANDLE procHandle = GetCurrentProcess();
  HANDLE threadHandle = GetCurrentThread();

  StackFrame frame(threadHandle);

  frame.m_Context.Eip = (DWORD)a_ProgramCounter;
  frame.m_Context.Ebp = (DWORD)a_AddressOfReturnAddress - 4;

  PRINT_VAR(GetLastErrorAsString());
  while (true) {
    bool success = StackWalk64(frame.m_ImageType, procHandle, threadHandle,
                               &frame.m_StackFrame, &frame.m_Context, nullptr,
                               &SymFunctionTableAccess64, &SymGetModuleBase64,
                               nullptr) != 0;
    if (!success) {
      PRINT_VAR(GetLastErrorAsString());
      break;
    }

    if (frame.m_StackFrame.AddrPC.Offset && depth < ORBIT_STACK_SIZE) {
      frame.m_Callstack.m_Data[depth++] = frame.m_StackFrame.AddrPC.Offset;
    } else {
      break;
    }
  }

  if (depth > 0) {
    frame.m_Callstack.m_Depth = depth;
    frame.m_Callstack.m_ThreadId = GetCurrentThreadId();
  }

  return frame.m_Callstack;
}

//-----------------------------------------------------------------------------
inline CallStackPOD CallStackPOD::GetCallstackManual(
    uint64_t a_ProgramCounter, uint64_t a_AddressOfReturnAddress) {
  CallStackPOD CS;

  CS.data_[CS.depth_++] = a_ProgramCounter;

  DWORD* Ebp = (DWORD*)((DWORD)a_AddressOfReturnAddress - 4);
  DWORD returnAddress = *(Ebp + 1);

  while (returnAddress && CS.depth_ < ORBIT_STACK_SIZE) {
    CS.data_[CS.depth_++] = returnAddress;
    Ebp = reinterpret_cast<DWORD*>(*Ebp);
    returnAddress = Ebp ? *(Ebp + 1) : NULL;
  }

  return CS;
}

#endif

#endif
