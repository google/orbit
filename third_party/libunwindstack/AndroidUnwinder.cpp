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

#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <android-base/stringprintf.h>
#include <android-base/threads.h>

#include <unwindstack/AndroidUnwinder.h>
#include <unwindstack/Arch.h>
#include <unwindstack/DexFiles.h>
#include <unwindstack/Error.h>
#include <unwindstack/JitDebug.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Regs.h>
#include <unwindstack/RegsGetLocal.h>
#include <unwindstack/Unwinder.h>

#if defined(__BIONIC__)
#include <bionic/reserved_signals.h>
static constexpr int kThreadUnwindSignal = BIONIC_SIGNAL_BACKTRACE;
#else
#include <signal.h>
static int kThreadUnwindSignal = SIGRTMIN;
#endif

// Use the demangler from libc++.
extern "C" char* __cxa_demangle(const char*, char*, size_t*, int* status);

namespace unwindstack {

void AndroidUnwinderData::DemangleFunctionNames() {
  for (auto& frame : frames) {
    char* demangled_name = __cxa_demangle(frame.function_name.c_str(), nullptr, nullptr, nullptr);
    if (demangled_name != nullptr) {
      frame.function_name = demangled_name;
      free(demangled_name);
    }
  }
}

std::string AndroidUnwinderData::GetErrorString() {
  std::string error_msg(GetErrorCodeString(error.code));
  if (error.address != 0) {
    error_msg += android::base::StringPrintf(" at address 0x%" PRIx64, error.address);
  }
  return error_msg;
}

AndroidUnwinder* AndroidUnwinder::Create(pid_t pid) {
  if (pid == getpid()) {
    return new AndroidLocalUnwinder;
  } else {
    return new AndroidRemoteUnwinder(pid);
  }
}

bool AndroidUnwinder::Initialize(ErrorData& error) {
  // Android stores the jit and dex file location only in the library
  // libart.so or libartd.so.
  static std::vector<std::string> search_libs [[clang::no_destroy]] = {"libart.so", "libartd.so"};

  bool initialize = true;
  std::call_once(initialize_, [this, &initialize, &error]() {
    initialize = InternalInitialize(error);
    if (!initialize) {
      return;
    }

    jit_debug_ = CreateJitDebug(arch_, process_memory_, search_libs);

#if defined(DEXFILE_SUPPORT)
    dex_files_ = CreateDexFiles(arch_, process_memory_, search_libs);
#endif
  });

  return initialize;
}

std::string AndroidUnwinder::FormatFrame(const FrameData& frame) const {
  if (arch_ == ARCH_UNKNOWN) {
    return "";
  }
  return Unwinder::FormatFrame(arch_, frame);
}

bool AndroidLocalUnwinder::InternalInitialize(ErrorData& error) {
  arch_ = Regs::CurrentArch();

  maps_.reset(new LocalUpdatableMaps);
  if (!maps_->Parse()) {
    error.code = ERROR_MAPS_PARSE;
    return false;
  }

  if (process_memory_ == nullptr) {
    process_memory_ = Memory::CreateProcessMemoryThreadCached(getpid());
  }

  return true;
}

FrameData AndroidUnwinder::BuildFrameFromPcOnly(uint64_t pc) {
  return Unwinder::BuildFrameFromPcOnly(pc, arch_, maps_.get(), jit_debug_.get(), process_memory_,
                                        true);
}

bool AndroidUnwinder::Unwind(AndroidUnwinderData& data) {
  return Unwind(std::nullopt, data);
}

bool AndroidUnwinder::Unwind(std::optional<pid_t> tid, AndroidUnwinderData& data) {
  if (!Initialize(data.error)) {
    return false;
  }

  return InternalUnwind(tid, data);
}

bool AndroidUnwinder::Unwind(void* ucontext, AndroidUnwinderData& data) {
  if (ucontext == nullptr) {
    data.error.code = ERROR_INVALID_PARAMETER;
    return false;
  }
  std::unique_ptr<Regs> regs(Regs::CreateFromUcontext(arch_, ucontext));
  return Unwind(regs.get(), data);
}

bool AndroidUnwinder::Unwind(Regs* initial_regs, AndroidUnwinderData& data) {
  if (initial_regs == nullptr) {
    data.error.code = ERROR_INVALID_PARAMETER;
    return false;
  }

  if (!Initialize(data.error)) {
    return false;
  }

  if (arch_ != initial_regs->Arch()) {
    data.error.code = ERROR_BAD_ARCH;
    return false;
  }

  std::unique_ptr<Regs> regs(initial_regs->Clone());
  if (data.saved_initial_regs) {
    (*data.saved_initial_regs).reset(initial_regs->Clone());
  }
  Unwinder unwinder(data.max_frames.value_or(max_frames_), maps_.get(), regs.get(),
                    process_memory_);
  unwinder.SetJitDebug(jit_debug_.get());
  unwinder.SetDexFiles(dex_files_.get());
  unwinder.Unwind(data.show_all_frames ? nullptr : &initial_map_names_to_skip_,
                  &map_suffixes_to_ignore_);
  data.frames = unwinder.ConsumeFrames();
  data.error = unwinder.LastError();
  return data.frames.size() != 0;
}

bool AndroidLocalUnwinder::InternalUnwind(std::optional<pid_t> tid, AndroidUnwinderData& data) {
  if (!tid) {
    *tid = android::base::GetThreadId();
  }

  if (static_cast<uint64_t>(*tid) == android::base::GetThreadId()) {
    // Unwind current thread.
    std::unique_ptr<Regs> regs(Regs::CreateFromLocal());
    RegsGetLocal(regs.get());
    return AndroidUnwinder::Unwind(regs.get(), data);
  }

  ThreadUnwinder unwinder(data.max_frames.value_or(max_frames_), maps_.get(), process_memory_);
  unwinder.SetJitDebug(jit_debug_.get());
  unwinder.SetDexFiles(dex_files_.get());
  std::unique_ptr<Regs>* initial_regs = nullptr;
  if (data.saved_initial_regs) {
    initial_regs = &data.saved_initial_regs.value();
  }
  unwinder.UnwindWithSignal(kThreadUnwindSignal, *tid, initial_regs,
                            data.show_all_frames ? nullptr : &initial_map_names_to_skip_,
                            &map_suffixes_to_ignore_);
  data.frames = unwinder.ConsumeFrames();
  data.error = unwinder.LastError();
  return data.frames.size() != 0;
}

bool AndroidRemoteUnwinder::InternalInitialize(ErrorData& error) {
  if (arch_ == ARCH_UNKNOWN) {
    arch_ = Regs::RemoteGetArch(pid_);
  }
  if (arch_ == ARCH_UNKNOWN) {
    error.code = ERROR_BAD_ARCH;
    return false;
  }

  maps_.reset(new RemoteMaps(pid_));
  if (!maps_->Parse()) {
    error.code = ERROR_MAPS_PARSE;
    return false;
  }

  if (process_memory_ == nullptr) {
    process_memory_ = Memory::CreateProcessMemoryCached(pid_);
  }

  return true;
}

bool AndroidRemoteUnwinder::InternalUnwind(std::optional<pid_t> tid, AndroidUnwinderData& data) {
  if (!tid) {
    *tid = pid_;
  }

  std::unique_ptr<Regs> regs(Regs::RemoteGet(*tid));
  return AndroidUnwinder::Unwind(regs.get(), data);
}

}  // namespace unwindstack
