//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "OrbitFunction.h"

#include <OrbitBase/Logging.h>

#include <map>

#include "Capture.h"
#include "ConnectionManager.h"
#include "Core.h"
#include "CoreApp.h"
#include "Log.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "Params.h"
#include "Pdb.h"
#include "PrintVar.h"
#include "SamplingProfiler.h"
#include "Serialization.h"
#include "TcpServer.h"
#include "Utils.h"

#ifdef _WIN32
#include <dia2.h>

#include "DiaParser.h"
#include "OrbitDia.h"
#include "SymbolUtils.h"
#endif

Function::Function() { ResetStats(); }

Function::Function(std::string_view name, std::string_view pretty_name,
                   std::string_view file, uint64_t address, uint64_t size,
                   uint64_t load_bias, Pdb* pdb)
    : name_(name),
      pretty_name_(pretty_name),
      file_(file),
      address_(address),
      size_(size),
      load_bias_(load_bias),
      pdb_(pdb) {
  CHECK(pdb != nullptr);
  ResetStats();
}

void Function::SetAsMainFrameFunction() {
  Capture::GMainFrameFunction = GetVirtualAddress();
  selected_ = true;
}

const std::string& Function::PrettyName() const {
  if (pretty_name_.empty()) {
    return name_;
  }

  return pretty_name_;
}

bool Function::Hookable() {
  if (Capture::IsLinuxData()) {
    return true;
  } else {
    // Don't allow hooking in asm implemented functions (strcpy, stccat...)
    // TODO: give this better thought.  Here is the theory:
    // Functions that loop back to first 5 bytes of instructions will explode as
    // the IP lands in the middle of the relative jump instruction...
    // Ideally, we would detect such a loop back and not allow hooking.
    if (file_.find(".asm") != std::string::npos) {
      return false;
    }

    CV_call_e conv = static_cast<CV_call_e>(calling_convention_);
    return ((conv == CV_CALL_NEAR_C || conv == CV_CALL_THISCALL) &&
            size_ >= 5) ||
           (GParams.m_AllowUnsafeHooking && size_ == 0);
  }
}

void Function::Select() {
  if (Hookable()) {
    selected_ = true;
    LOG("Selected %s at 0x%" PRIx64 " (address_=0x%" PRIx64
        ", load_bias_= 0x%" PRIx64 ", base_address=0x%" PRIx64 ")",
        pretty_name_.c_str(), GetVirtualAddress(), address_, load_bias_,
        module_base_address_);
    Capture::GSelectedFunctionsMap[GetVirtualAddress()] = this;
  }
}

void Function::UnSelect() {
  selected_ = false;
  Capture::GSelectedFunctionsMap.erase(GetVirtualAddress());
}

void Function::SetPdb(Pdb* pdb) {
  pdb_ = pdb;
  module_base_address_ = pdb_->GetHModule();
  loaded_module_name_ = pdb_->GetLoadedModuleName();
}

uint64_t Function::GetVirtualAddress() const {
  return address_ + module_base_address_ - load_bias_;
}

uint64_t Function::Offset() const { return address_ - load_bias_; }

std::string Function::GetModuleName() const {
  if (pdb_ != nullptr) {
    return pdb_->GetName();
  } else {
    // TODO: does it assume that address is absolute - if so this is wrong
    // the address here is always a vaddr (at least on Linux)
    std::shared_ptr<Module> module =
        Capture::GTargetProcess->GetModuleFromAddress(address_);
    return module ? module->m_Name : "";
  }
}

class Type* Function::GetParentType() {
  return parent_id_ != 0 ? &pdb_->GetTypeFromId(parent_id_) : nullptr;
}

void Function::ResetStats() {
  if (stats_ == nullptr) {
    stats_ = std::make_shared<FunctionStats>();
  } else {
    stats_->Reset();
  }
}

void Function::UpdateStats(const Timer& timer) {
  if (stats_ != nullptr) {
    stats_->Update(timer);
  }
}

void Function::GetDisassembly(uint32_t pid) {
  GCoreApp->GetRemoteMemory(
      pid, GetVirtualAddress(), Size(), [this](std::vector<uint8_t>& data) {
        GCoreApp->Disassemble(pretty_name_, GetVirtualAddress(), data.data(),
                              data.size());
      });
}

void Function::FindFile() {
#ifdef _WIN32
  LineInfo lineInfo;
  SymUtils::GetLineInfo(GetVirtualAddress(), lineInfo);
  if (lineInfo.m_File != "") file_ = lineInfo.m_File;
  file_ = ToLower(file_);
  line_ = lineInfo.m_Line;
#endif
}

const char* Function::GetCallingConventionString() {
  static const char* kCallingConventions[] = {
      "NEAR_C",       // CV_CALL_NEAR_C      = 0x00, // near right to left push,
                      // caller pops stack
      "FAR_C",        // CV_CALL_FAR_C       = 0x01, // far right to left push,
                      // caller pops stack
      "NEAR_PASCAL",  // CV_CALL_NEAR_PASCAL = 0x02, // near left to right
                      // push, callee pops stack
      "FAR_PASCAL",   // CV_CALL_FAR_PASCAL  = 0x03, // far left to right push,
                      // callee pops stack
      "NEAR_FAST",    // CV_CALL_NEAR_FAST   = 0x04, // near left to right push
                      // with regs, callee pops stack
      "FAR_FAST",     // CV_CALL_FAR_FAST    = 0x05, // far left to right push
                      // with regs, callee pops stack
      "SKIPPED",   // CV_CALL_SKIPPED     = 0x06, // skipped (unused) call index
      "NEAR_STD",  // CV_CALL_NEAR_STD    = 0x07, // near standard call
      "FAR_STD",   // CV_CALL_FAR_STD     = 0x08, // far standard call
      "NEAR_SYS",  // CV_CALL_NEAR_SYS    = 0x09, // near sys call
      "FAR_SYS",   // CV_CALL_FAR_SYS     = 0x0a, // far sys call
      "THISCALL",  // CV_CALL_THISCALL    = 0x0b, // this call (this passed in
                   // register)
      "MIPSCALL",  // CV_CALL_MIPSCALL    = 0x0c, // Mips call
      "GENERIC",   // CV_CALL_GENERIC     = 0x0d, // Generic call sequence
      "ALPHACALL",  // CV_CALL_ALPHACALL   = 0x0e, // Alpha call
      "PPCCALL",    // CV_CALL_PPCCALL     = 0x0f, // PPC call
      "SHCALL",     // CV_CALL_SHCALL      = 0x10, // Hitachi SuperH call
      "ARMCALL",    // CV_CALL_ARMCALL     = 0x11, // ARM call
      "AM33CALL",   // CV_CALL_AM33CALL    = 0x12, // AM33 call
      "TRICALL",    // CV_CALL_TRICALL     = 0x13, // TriCore Call
      "SH5CALL",    // CV_CALL_SH5CALL     = 0x14, // Hitachi SuperH-5 call
      "M32RCALL",   // CV_CALL_M32RCALL    = 0x15, // M32R Call
      "CLRCALL",    // CV_CALL_CLRCALL     = 0x16, // clr call
      "INLINE",     // CV_CALL_INLINE      = 0x17, // Marker for routines always
                    // inlined and thus lacking a convention
      "NEAR_VECTOR",  // CV_CALL_NEAR_VECTOR = 0x18, // near left to right push
                      // with regs, callee pops stack
      "RESERVED"};    // CV_CALL_RESERVED    = 0x19  // first unused call
                      // enumeration
  if (calling_convention_ < 0 ||
      static_cast<size_t>(calling_convention_) >=
          (sizeof(kCallingConventions) / sizeof(kCallingConventions[0]))) {
    return "UnknownCallConv";
  }

  return kCallingConventions[calling_convention_];
}

ORBIT_SERIALIZE(Function, 3) {
  ORBIT_NVP_VAL(0, name_);
  ORBIT_NVP_VAL(0, pretty_name_);
  ORBIT_NVP_VAL(0, address_);
  ORBIT_NVP_VAL(0, size_);
  ORBIT_NVP_VAL(0, load_bias_);
  ORBIT_NVP_VAL(0, module_);
  ORBIT_NVP_VAL(0, file_);
  ORBIT_NVP_VAL(0, line_);
  ORBIT_NVP_VAL(0, calling_convention_);
  ORBIT_NVP_VAL(1, stats_);
  ORBIT_NVP_VAL(2, probe_);
  ORBIT_NVP_VAL(3, module_base_address_);
  ORBIT_NVP_VAL(3, loaded_module_name_);
}

FunctionParam::FunctionParam() {
#ifdef _WIN32
  memset(&m_SymbolInfo, 0, sizeof(m_SymbolInfo));
#endif
}

bool FunctionParam::InRegister(int a_Index) { return a_Index < 4; }

bool FunctionParam::IsFloat() {
  return (m_Type.find("float") != std::string::npos ||
          m_Type.find("double") != std::string::npos);
}

void Function::ProcessArgumentInfo() {
#ifdef _WIN32
  arguments_.clear();
  arguments_.reserve(params_.size());

  int argIndex = IsMemberFunction() ? 1 : 0;

  for (FunctionParam& param : params_) {
    Argument arg;
    arg.m_Index = argIndex;
    arg.m_Reg = (CV_HREG_e)param.m_SymbolInfo.Register;
    arg.m_Offset = (DWORD)param.m_SymbolInfo.Address;
    arg.m_NumBytes = param.m_SymbolInfo.Size;
    arguments_.push_back(arg);

    ++argIndex;
  }
#endif
}

bool Function::IsMemberFunction() {
  // TODO
  return false;
}

void Function::Print() {
#ifdef _WIN32
  if (pdb_ == nullptr) {
    return;
  }

  std::shared_ptr<OrbitDiaSymbol> diaSymbol = pdb_->GetDiaSymbolFromId(id_);
  if (diaSymbol->m_Symbol) {
    OrbitDia::DiaDump(diaSymbol->m_Symbol);

    if (parent_id_ != 0) {
      class Type& type = pdb_->GetTypeFromId(parent_id_);
      type.LoadDiaInfo();
      type.GenerateDiaHierarchy();
    }

    DiaParser parser;
    parser.PrintFunctionType(diaSymbol->m_Symbol);
    ORBIT_VIZ(ws2s(parser.m_Log));
  }

  LineInfo lineInfo;
  SymUtils::GetLineInfo(address_ + module_base_address_, lineInfo);
  ORBIT_VIZV(lineInfo.m_File);
  ORBIT_VIZV(lineInfo.m_Line);
  ORBIT_VIZV(lineInfo.m_Address);
#endif

  ORBIT_VIZV(address_);
  ORBIT_VIZV(selected_);

  if (!params_.empty()) {
    ORBIT_VIZ("\nParams:");
    for (auto& var : params_) {
      ORBIT_VIZV(var.m_Name);
      ORBIT_VIZV(var.m_Address);
      ORBIT_VIZV(var.m_ParamType);
      ORBIT_VIZV(var.m_Type);
    }
  }
}
