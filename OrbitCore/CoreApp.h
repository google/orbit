//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "BaseTypes.h"
#include "SymbolsManager.h"
#include "TransactionManager.h"

#ifdef _WIN32
#include <windows.h>
#endif

class ModuleDebugInfo;
class Timer;
class LinuxCallstackEvent;
struct CallStack;
struct ContextSwitch;
struct CallstackEvent;
class Session;

class CoreApp {
 public:
  virtual void InitializeManagers();
  virtual void SendToUiAsync(const std::wstring& /*a_Msg*/) {}
  virtual void SendToUiNow(const std::wstring& /*a_Msg*/) {}
  virtual bool GetUnrealSupportEnabled() { return false; }
  virtual bool GetUnitySupportEnabled() { return false; }
  virtual bool GetUnsafeHookingEnabled() { return false; }
  virtual bool GetSamplingEnabled() { return false; }
  virtual bool GetOutputDebugStringEnabled() { return false; }
  virtual void LogMsg(const std::wstring& /*a_Msg*/) {}
  virtual void UpdateVariable(class Variable* /*a_Variable*/) {}
  virtual void Disassemble(const std::string& /*a_FunctionName*/,
                           DWORD64 /*a_VirtualAddress*/,
                           const char* /*a_MachineCode*/, size_t /*a_Size*/) {}
  virtual void ProcessTimer(const Timer& /*a_Timer*/,
                            const std::string& /*a_FunctionName*/) {}
  virtual void ProcessSamplingCallStack(LinuxCallstackEvent& /*a_CS*/) {}
  virtual void ProcessHashedSamplingCallStack(CallstackEvent& /*a_CallStack*/) {
  }
  virtual void ProcessCallStack(CallStack& /*a_CallStack*/) {}
  virtual void ProcessContextSwitch(const ContextSwitch& /*a_ContextSwitch*/) {}
  virtual void AddSymbol(uint64_t /*a_Address*/,
                         const std::string& /*a_Module*/,
                         const std::string& /*a_Name*/) {}
  virtual void AddKeyAndString(uint64_t key, std::string_view str) {}
  virtual bool SelectProcess(const std::string& a_Process) { return false; }
  virtual void OnRemoteModuleDebugInfo(const std::vector<ModuleDebugInfo>&) {}
  virtual void ApplySession(std::shared_ptr<Session> session) {};
  virtual const std::unordered_map<DWORD64, std::shared_ptr<class Rule> >*
  GetRules() {
    return nullptr;
  }
  virtual void RefreshCaptureView() {}

  // Managers
  std::shared_ptr<orbit::TransactionManager> GetTransactionManager() {
    return transaction_manager_;
  }
  std::shared_ptr<orbit::SymbolsManager> GetSymbolsManager() {
    return symbols_manager_;
  }

  private:
    std::shared_ptr<orbit::TransactionManager> transaction_manager_ = nullptr;
    std::shared_ptr<orbit::SymbolsManager> symbols_manager_ = nullptr;
};

extern std::shared_ptr<CoreApp> GCoreApp;
