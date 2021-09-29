// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LIB_ORBIT_LIB_H_
#define ORBIT_LIB_ORBIT_LIB_H_

#include <stdint.h>

#ifdef ORBIT_LIB_API_EXPORTS
#define ORBIT_LIB_API __declspec(dllexport)
#else
#define ORBIT_LIB_API __declspec(dllimport)
#endif

namespace orbit_lib {

struct ErrorHandler {
  virtual void OnError(const char* error_message) = 0;
};

struct ProcessListener : public ErrorHandler {
  virtual void OnProcess(const char* process_path, uint32_t pid, bool is_64_bit,
                         float cpu_usage) = 0;
};

struct ModuleListener : public ErrorHandler {
  virtual void OnModule(const char* module_path, uint64_t start_address, uint64_t end_address,
                        uint64_t file_size) = 0;
};

struct DebugInfoListener : public ErrorHandler {
  virtual void OnFunction(const char* module_path, const char* function_name,
                          uint64_t relative_address, uint64_t size, const char* file_name,
                          int line) = 0;
};

struct CaptureListener : public ErrorHandler {
  virtual void OnTimer(uint64_t absolute_address, uint64_t start, uint64_t end, uint32_t tid,
                       uint32_t pid) = 0;
};

ORBIT_LIB_API int ListProcesses(ProcessListener* listener);
ORBIT_LIB_API int ListModules(uint32_t pid, ModuleListener* listener);
ORBIT_LIB_API int ListFunctions(const char* symbols_path, DebugInfoListener* listener);

struct FunctionHook {
  enum class Type { kInvalid, kRegular, kFileIO };
  uint64_t address = 0;
  Type type = Type::kInvalid;
};

ORBIT_LIB_API int StartCapture(uint32_t pid, FunctionHook* function_hooks, size_t num_hooks,
                               CaptureListener* listener);
ORBIT_LIB_API int StopCapture();

}  // namespace orbit_lib

#endif  // ORBIT_LIB_ORBIT_LIB_H_