// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "BaseTypes.h"
#include "LinuxAddressInfo.h"
#include "LinuxCallstackEvent.h"
#include "ProcessMemoryClient.h"
#include "SymbolsClient.h"
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"

#ifdef _WIN32
#include <windows.h>
#endif

struct ModuleDebugInfo;
class Timer;
struct CallStack;
struct ContextSwitch;
struct CallstackEvent;
class Session;

class CoreApp {
 public:
  virtual ~CoreApp() = default;
  virtual void InitializeClientTransactions();
  virtual void SendToUiAsync(const std::string& /*message*/) {}
  virtual void SendToUiNow(const std::string& /*message*/) {}
  virtual bool GetUnrealSupportEnabled() { return false; }
  virtual bool GetUnitySupportEnabled() { return false; }
  virtual bool GetUnsafeHookingEnabled() { return false; }
  virtual bool GetSamplingEnabled() { return false; }
  virtual bool GetOutputDebugStringEnabled() { return false; }
  virtual bool GetUploadDumpsToServerEnabled() const { return false; }
  virtual void UpdateVariable(class Variable* /*a_Variable*/) {}
  virtual void Disassemble(const std::string& /*a_FunctionName*/,
                           uint64_t /*a_VirtualAddress*/,
                           const uint8_t* /*a_MachineCode*/,
                           size_t /*a_Size*/) {}
  virtual void ProcessTimer(const Timer& /*a_Timer*/,
                            const std::string& /*a_FunctionName*/) {}
  virtual void ProcessSamplingCallStack(LinuxCallstackEvent& /*a_CS*/) {}
  virtual void ProcessHashedSamplingCallStack(CallstackEvent& /*a_CallStack*/) {
  }
  virtual void ProcessContextSwitch(const ContextSwitch& /*a_ContextSwitch*/) {}
  virtual void AddAddressInfo(LinuxAddressInfo /*address_info*/) {}
  virtual void AddKeyAndString(uint64_t /*key*/, std::string_view /*str*/) {}
  virtual void OnRemoteModuleDebugInfo(const std::vector<ModuleDebugInfo>&) {}
  virtual void ApplySession(const Session&) {}
  virtual const std::unordered_map<DWORD64, std::shared_ptr<class Rule> >*
  GetRules() {
    return nullptr;
  }
  virtual void RefreshCaptureView() {}

  using MemoryCallback = ProcessMemoryClient::ProcessMemoryCallback;
  void GetRemoteMemory(uint32_t pid, uint64_t address, uint64_t size,
                       const MemoryCallback& callback) {
    process_memory_client_->GetRemoteMemory(pid, address, size, callback);
  }

 protected:
  TransactionClient* GetTransactionClient() {
    return transaction_client_.get();
  }
  SymbolsClient* GetSymbolsClient() { return symbols_client_.get(); }

 private:
  std::unique_ptr<TransactionClient> transaction_client_;
  std::unique_ptr<SymbolsClient> symbols_client_;
  std::unique_ptr<ProcessMemoryClient> process_memory_client_;
};

extern CoreApp* GCoreApp;
