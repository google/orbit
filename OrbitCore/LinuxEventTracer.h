//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------
#pragma once

#include <atomic>
#include <functional>
#include <thread>

#include "CallstackTypes.h"
#include "LibunwindstackUnwinder.h"

//-----------------------------------------------------------------------------
class LinuxEventTracer {
 public:
  static const uint32_t ROUND_ROBIN_BATCH_SIZE = 5;
  using Callback = std::function<void(const std::string& a_Data)>;

  explicit LinuxEventTracer(pid_t a_Pid, uint32_t a_Frequency = 1000)
      : m_Pid{a_Pid}, m_Frequency{a_Frequency} {}

  void Start();
  void Stop();
  bool IsRunning() const { return !(*m_ExitRequested); }

 protected:
  void CommandCallback(std::string a_Line);

 private:
  pid_t m_Pid;
  uint32_t m_Frequency;

  // m_ExitRequested must outlive this object because it is used by m_Thread.
  // The control block of shared_ptr is thread safe (i.e., reference counting
  // and pointee's lifetime management are atomic and thread safe).
  std::shared_ptr<std::atomic<bool>> m_ExitRequested =
      std::make_unique<std::atomic<bool>>(true);
  std::shared_ptr<std::thread> m_Thread;
  Callback m_Callback;

  static void Run(pid_t a_Pid, int32_t a_Frequency,
                  const std::shared_ptr<std::atomic<bool>>& a_ExitRequested);
  static void HandleCallstack(
      ThreadID tid, uint64_t timestamp,
      const std::vector<unwindstack::FrameData>& frames);
};