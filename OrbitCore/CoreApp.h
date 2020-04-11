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
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"

#ifdef _WIN32
#include <windows.h>
#endif

struct ModuleDebugInfo;
class Timer;
class LinuxCallstackEvent;
struct CallStack;
struct ContextSwitch;
struct CallstackEvent;
class Session;

class CoreApp {
 public:
  virtual ~CoreApp() = default;
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
  virtual void AddKeyAndString(uint64_t /*key*/, std::string_view /*str*/) {}
  virtual void OnRemoteModuleDebugInfo(const std::vector<ModuleDebugInfo>&) {}
  virtual void ApplySession(std::shared_ptr<Session> /*session*/){};
  virtual const std::unordered_map<DWORD64, std::shared_ptr<class Rule> >*
  GetRules() {
    return nullptr;
  }
  virtual void RefreshCaptureView() {}

  void GetRemoteMemory(uint64_t address, uint64_t size,
                       std::function<void(std::vector<byte>&)> callback);

  // Managers
  orbit::TransactionManager* GetTransactionManager() {
    return transaction_manager_.get();
  }
  orbit::SymbolsManager* GetSymbolsManager() { return symbols_manager_.get(); }

  bool IsClient() const { return is_client_; }
  bool IsService() const { return is_service_; }

 private:
  // Transactions
  void SetupMemoryTransaction();
  typedef std::function<void(std::vector<byte>&)> memory_callback;
  absl::flat_hash_map<uint64_t, memory_callback> memory_callbacks_;
  absl::Mutex transaction_mutex_;

  std::unique_ptr<orbit::TransactionManager> transaction_manager_ = nullptr;
  std::unique_ptr<orbit::SymbolsManager> symbols_manager_ = nullptr;

  bool is_client_;
  bool is_service_;
};

extern CoreApp* GCoreApp;
