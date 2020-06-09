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
  virtual void SendToUi(const std::string& /* message */) {}
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
  virtual void ProcessTimer(const Timer& /*timer*/) {}
  virtual void ProcessSamplingCallStack(LinuxCallstackEvent& /*a_CS*/) {}
  virtual void ProcessHashedSamplingCallStack(CallstackEvent& /*a_CallStack*/) {
  }
  virtual void AddAddressInfo(LinuxAddressInfo /*address_info*/) {}
  virtual void AddKeyAndString(uint64_t /*key*/, std::string_view /*str*/) {}
  virtual void UpdateThreadName(uint32_t /*thread_id*/,
                                const std::string& /*thread_name*/) {}
  virtual void OnRemoteModuleDebugInfo(const std::vector<ModuleDebugInfo>&) {}
  virtual void ApplySession(const Session&) {}
  virtual void RefreshCaptureView() {}

  using ProcessMemoryCallback =
      std::function<void(const std::vector<uint8_t>&)>;
  virtual void GetRemoteMemory(uint32_t /*pid*/, uint64_t /*address*/,
                               uint64_t /*size*/,
                               const ProcessMemoryCallback& /*callback*/) {}
};

extern CoreApp* GCoreApp;
