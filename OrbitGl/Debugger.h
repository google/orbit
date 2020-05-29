/*
 * Copyright (c) 2020 The Orbit Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#pragma once

#include <atomic>
#include <string>
#include <vector>

#include "Message.h"

class Debugger {
 public:
  Debugger();
  ~Debugger();

  void LaunchProcess(const std::string& process_name,
                     const std::string& working_dir, const std::string& args);
  void MainTick();
  void SendThawMessage();

 protected:
  void DebuggerThread(const std::string& process_name,
                      const std::string& working_dir, const std::string& args);

 private:
  OrbitWaitLoop m_WaitLoop;
  std::atomic<bool> m_LoopReady;
  DWORD m_ProcessID;
};
