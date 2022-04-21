/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <unwindstack/Arch.h>
#include <unwindstack/DexFiles.h>
#include <unwindstack/Error.h>
#include <unwindstack/JitDebug.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Regs.h>
#include <unwindstack/SharedString.h>
#include <unwindstack/Unwinder.h>

namespace unwindstack {

struct AndroidUnwinderData {
  AndroidUnwinderData() = default;
  explicit AndroidUnwinderData(const size_t max_frames) : max_frames(max_frames) {}
  explicit AndroidUnwinderData(const bool show_all_frames) : show_all_frames(show_all_frames) {}

  void DemangleFunctionNames();

  std::string GetErrorString();

  std::vector<FrameData> frames;
  ErrorData error;
  std::optional<std::unique_ptr<Regs>> saved_initial_regs;
  const std::optional<size_t> max_frames = std::nullopt;
  const bool show_all_frames = false;
};

class AndroidUnwinder {
 public:
  AndroidUnwinder(pid_t pid) : pid_(pid) {}
  AndroidUnwinder(pid_t pid, std::shared_ptr<Memory>& memory)
      : pid_(pid), process_memory_(memory) {}
  AndroidUnwinder(pid_t pid, ArchEnum arch) : pid_(pid), arch_(arch) {}
  AndroidUnwinder(pid_t pid, const std::vector<std::string> initial_map_names_to_skip)
      : pid_(pid), initial_map_names_to_skip_(std::move(initial_map_names_to_skip)) {}
  AndroidUnwinder(pid_t pid, const std::vector<std::string> initial_map_names_to_skip,
                  const std::vector<std::string> map_suffixes_to_ignore)
      : pid_(pid),
        initial_map_names_to_skip_(std::move(initial_map_names_to_skip)),
        map_suffixes_to_ignore_(std::move(map_suffixes_to_ignore)) {}
  virtual ~AndroidUnwinder() = default;

  bool Initialize(ErrorData& error);

  std::shared_ptr<Memory>& GetProcessMemory() { return process_memory_; }
  unwindstack::Maps* GetMaps() { return maps_.get(); }

  const JitDebug& GetJitDebug() { return *jit_debug_.get(); }
  const DexFiles& GetDexFiles() { return *dex_files_.get(); }

  std::string FormatFrame(const FrameData& frame) const;

  bool Unwind(AndroidUnwinderData& data);
  bool Unwind(std::optional<pid_t> tid, AndroidUnwinderData& data);
  bool Unwind(void* ucontext, AndroidUnwinderData& data);
  bool Unwind(Regs* initial_regs, AndroidUnwinderData& data);

  FrameData BuildFrameFromPcOnly(uint64_t pc);

  static AndroidUnwinder* Create(pid_t pid);

 protected:
  virtual bool InternalInitialize(ErrorData& error) = 0;

  virtual bool InternalUnwind(std::optional<pid_t> tid, AndroidUnwinderData& data) = 0;

  pid_t pid_;

  size_t max_frames_ = kMaxNumFrames;
  std::vector<std::string> initial_map_names_to_skip_;
  std::vector<std::string> map_suffixes_to_ignore_;
  std::once_flag initialize_;

  ArchEnum arch_ = ARCH_UNKNOWN;

  std::shared_ptr<Maps> maps_;
  std::shared_ptr<Memory> process_memory_;
  std::unique_ptr<JitDebug> jit_debug_;
  std::unique_ptr<DexFiles> dex_files_;

  static constexpr size_t kMaxNumFrames = 512;
};

class AndroidLocalUnwinder : public AndroidUnwinder {
 public:
  AndroidLocalUnwinder() : AndroidUnwinder(getpid()) {
    initial_map_names_to_skip_.emplace_back(kUnwindstackLib);
  }
  AndroidLocalUnwinder(std::shared_ptr<Memory>& process_memory)
      : AndroidUnwinder(getpid(), process_memory) {
    initial_map_names_to_skip_.emplace_back(kUnwindstackLib);
  }
  AndroidLocalUnwinder(const std::vector<std::string>& initial_map_names_to_skip)
      : AndroidUnwinder(getpid(), initial_map_names_to_skip) {
    initial_map_names_to_skip_.emplace_back(kUnwindstackLib);
  }
  AndroidLocalUnwinder(const std::vector<std::string>& initial_map_names_to_skip,
                       const std::vector<std::string>& map_suffixes_to_ignore)
      : AndroidUnwinder(getpid(), initial_map_names_to_skip, map_suffixes_to_ignore) {
    initial_map_names_to_skip_.emplace_back(kUnwindstackLib);
  }
  virtual ~AndroidLocalUnwinder() = default;

 protected:
  static constexpr const char* kUnwindstackLib = "libunwindstack.so";

  bool InternalInitialize(ErrorData& error) override;

  bool InternalUnwind(std::optional<pid_t> tid, AndroidUnwinderData& data) override;
};

class AndroidRemoteUnwinder : public AndroidUnwinder {
 public:
  AndroidRemoteUnwinder(pid_t pid) : AndroidUnwinder(pid) {}
  AndroidRemoteUnwinder(pid_t pid, std::shared_ptr<Memory>& process_memory)
      : AndroidUnwinder(pid, process_memory) {}
  AndroidRemoteUnwinder(pid_t pid, ArchEnum arch) : AndroidUnwinder(pid, arch) {}
  AndroidRemoteUnwinder(pid_t pid, const std::vector<std::string> initial_map_names_to_skip)
      : AndroidUnwinder(pid, initial_map_names_to_skip) {}
  AndroidRemoteUnwinder(pid_t pid, const std::vector<std::string> initial_map_names_to_skip,
                        const std::vector<std::string> map_suffixes_to_ignore)
      : AndroidUnwinder(pid, initial_map_names_to_skip, map_suffixes_to_ignore) {}
  virtual ~AndroidRemoteUnwinder() = default;

 protected:
  bool InternalInitialize(ErrorData& error) override;

  bool InternalUnwind(std::optional<pid_t> tid, AndroidUnwinderData& data) override;
};

}  // namespace unwindstack
