//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Callstack.h"

#include "Capture.h"
#include "Core.h"
#include "OrbitProcess.h"
#include "OrbitType.h"
#include "PrintVar.h"
#include "Serialization.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
CallStack::CallStack(CallStackPOD a_CS) : CallStack() {
  m_Hash = a_CS.Hash();
  m_ThreadId = a_CS.ThreadId();
  m_Depth = a_CS.Depth();
  m_Data.resize(m_Depth);
  memcpy(m_Data.data(), a_CS.Data(), m_Depth * sizeof(m_Data[0]));
}

//-----------------------------------------------------------------------------
void CallStack::Print() {
  PRINT_VAR(m_Hash);
  PRINT_VAR(m_Depth);
  PRINT_VAR(m_ThreadId);

  for (uint32_t i = 0; i < m_Depth; ++i) {
    std::string address = VAR_TO_STR((void*)m_Data[i]);
    PRINT_VAR(address);
  }
}

//-----------------------------------------------------------------------------
std::string CallStack::GetString() {
  std::string callstackString;

  ScopeLock lock(Capture::GTargetProcess->GetDataMutex());
  for (uint32_t i = 0; i < m_Depth; ++i) {
    DWORD64 addr = m_Data[i];
    Function* func =
        Capture::GTargetProcess->GetFunctionFromAddress(addr, false);

    if (func) {
      callstackString += func->PrettyName() + "\n";
    } else {
      callstackString += absl::StrFormat("%" PRIx64 "\n", addr);
    }
  }

  return callstackString;
}

#ifdef _WIN32
//-----------------------------------------------------------------------------
StackFrame::StackFrame(HANDLE a_Thread) {
  // http://www.codeproject.com/threads/StackWalker.asp

  STACKFRAME64& s = m_StackFrame;
  CONTEXT& c = m_Context;

  memset(this, 0, sizeof(*this));
  c.ContextFlags = CONTEXT_FULL;

  GetThreadContext(a_Thread, &m_Context);

#ifdef _M_IX86
  // normally, call ImageNtHeader() and use machine info from PE header
  m_ImageType = IMAGE_FILE_MACHINE_I386;
  s.AddrPC.Offset = c.Eip;
  s.AddrPC.Mode = AddrModeFlat;
  s.AddrFrame.Offset = c.Ebp;
  s.AddrFrame.Mode = AddrModeFlat;
  s.AddrStack.Offset = c.Esp;
  s.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
  m_ImageType = IMAGE_FILE_MACHINE_AMD64;
  s.AddrPC.Offset = c.Rip;
  s.AddrPC.Mode = AddrModeFlat;
  s.AddrFrame.Offset = c.Rsp;
  s.AddrFrame.Mode = AddrModeFlat;
  s.AddrStack.Offset = c.Rsp;
  s.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
  m_ImageType = IMAGE_FILE_MACHINE_IA64;
  s.AddrPC.Offset = c.StIIP;
  s.AddrPC.Mode = AddrModeFlat;
  s.AddrFrame.Offset = c.IntSp;
  s.AddrFrame.Mode = AddrModeFlat;
  s.AddrBStore.Offset = c.RsBSP;
  s.AddrBStore.Mode = AddrModeFlat;
  s.AddrStack.Offset = c.IntSp;
  s.AddrStack.Mode = AddrModeFlat;
#else
#warning "Platform not supported!"
#endif
}

//-----------------------------------------------------------------------------
__declspec(noinline) CallStackPOD CallStackPOD::Walk(uint64_t a_Rip,
                                                     uint64_t a_Rsp) {
  CallStackPOD callstack;

#ifdef _WIN64
  CONTEXT Context;
  KNONVOLATILE_CONTEXT_POINTERS NvContext;
  UNWIND_HISTORY_TABLE UnwindHistoryTable;
  PRUNTIME_FUNCTION RuntimeFunction;
  PVOID HandlerData;
  ULONG64 EstablisherFrame;
  ULONG64 ImageBase;

  RtlCaptureContext(&Context);

  if (a_Rip != 0) Context.Rip = a_Rip;
  if (a_Rsp != 0) Context.Rsp = a_Rsp;

  RtlZeroMemory(&UnwindHistoryTable, sizeof(UNWIND_HISTORY_TABLE));

  callstack.data_[callstack.depth_++] = a_Rip;

  for (ULONG Frame = 0;; Frame++) {
    RuntimeFunction =
        RtlLookupFunctionEntry(Context.Rip, &ImageBase, &UnwindHistoryTable);

    RtlZeroMemory(&NvContext, sizeof(KNONVOLATILE_CONTEXT_POINTERS));

    if (!RuntimeFunction) {
      //
      // If we don't have a RUNTIME_FUNCTION, then we've encountered
      // a leaf function.  Adjust the stack appropriately.
      //

      Context.Rip = (ULONG64)(*(PULONG64)Context.Rsp);
      Context.Rsp += 8;
    } else {
      RtlVirtualUnwind(UNW_FLAG_NHANDLER, ImageBase, Context.Rip,
                       RuntimeFunction, &Context, &HandlerData,
                       &EstablisherFrame, &NvContext);
    }

    if (!Context.Rip) break;

    if (callstack.depth_ < ORBIT_STACK_SIZE) {
      callstack.data_[callstack.depth_++] = Context.Rip;
    }
  }

  callstack.CalculateHash();

#else
  callstack = GetCallstackManual(a_Rip, a_Rsp);
#endif

  return callstack;
}

#endif

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(CallStack, 0) {
  ORBIT_NVP_VAL(0, m_Data);
  ORBIT_NVP_VAL(0, m_Hash);
  ORBIT_NVP_VAL(0, m_Depth);
  ORBIT_NVP_VAL(0, m_ThreadId);
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(HashedCallStack, 0) {
  ORBIT_NVP_VAL(0, m_Hash);
  ORBIT_NVP_VAL(0, m_ThreadId);
}
