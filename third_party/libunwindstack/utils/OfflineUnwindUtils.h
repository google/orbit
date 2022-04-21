/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <unwindstack/Arch.h>
#include <unwindstack/JitDebug.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Regs.h>
#include <unwindstack/Unwinder.h>

#include "MemoryOffline.h"

// These utils facilitate performing offline unwinds. Offline unwinds are similar to local
// unwinds, however, instead of pausing the process to gather the current execution state
// (stack, registers, Elf / maps), a snapshot of the process is taken. This snapshot data
// is used at a later time (when the process is no longer running) to unwind the process
// at the point the snapshot was taken.
//
// Offline unwinds simulate one of the most common use cases of the Unwinder. These types of
// unwinds are performed by two of the largest clients of libunwindstack: Perfetto and Simpleperf.
//
// Offline unwind snapshots were obtained using the following approach:
// 1. (Optional) Flash a virtual or physical device with the internal Android build rather than
//    an AOSP build to have additional and more complex apps to unwind.
// 2. Determine the pid of the app/process you want to unwind. View all of the running
//    processes with `adb shell ps -A`  or `adb shell ps -A | grep name.of.process` if you know
//    the (package) name of the process.
// 3. (Optional) If you want to ensure that an application is compiled or that the compiled code is
//    erased (e.g. want interpreter / JIT frames in the unwind), run `adb shell cmd package compile`
//    based on the options provided here
//    (https://source.android.com/devices/tech/dalvik/jit-compiler).
// 4. Ensure the process is running and in a "desired state" when you execute
//    `adb shell /bin/unwind_for_offline [options] pid`. For example:
//   a. If you are unwinding the bluetooth process and want the unwind to contain the bluetooth
//      ELF (libbluetooth.so), try to pair with a device over bluetooth. Make sure you use the
//      `-t` and `-e` flags.
//   b. You will likely see more variation in the thread snapshots (especially if you are trying
//      to capture JIT/interpreter frames) if you ensure the app is not-idle when you run
//      `unwind_for_offline`. E.g. immediately run unwind_for_offline after searching for a
//      landmark in Google Maps.
// 5. Grab the desired snapshot directories with `adb pull ...`
// 6. (Optional) Reduce the size of copied ELFs:
//   a. Use tools/share_common_elfs.sh to eliminate copies of the same ELF files that are already
//      used by other 'libunwindstack/offline_files/' subdirectories.
//   b. Strip ELFs of all sections that are not needed for unwinding and/or symbolization.
//   c. Compress/Zip the entire snapshot directory.
// 7. Use the path to the snapshot directory(ies) for the `offline_files_dirs` parameter to
//    `OfflineUnwindUtils::Init`.
//
// See b/192012600 for additional information regarding Offline Unwind Benchmarks.
namespace unwindstack {

void DecompressFiles(const std::string& directory);

std::string GetOfflineFilesDirectory();

std::string DumpFrames(const Unwinder& unwinder);

bool AddMemory(std::string file_name, MemoryOfflineParts* parts, std::string* error_msg);

// Enum that indicates how `UnwindSample::process_memory` of `OfflineUnwindUtils::samples_`
// should be initialized.
enum class ProcessMemoryFlag {
  kNone = 0,
  kIncludeJitMemory,
  kNoMemory,
};

// A `UnwindSampleInfo` object contains the information necessary for OfflineUnwindUtils
// to initialize a single offline unwind sample.
struct UnwindSampleInfo {
  std::string offline_files_dir;
  ArchEnum arch;
  std::string frame_info_filename = "output.txt";
  ProcessMemoryFlag memory_flag = ProcessMemoryFlag::kNone;
  bool create_maps = true;
};

// The `OfflineUnwindUtils` class helps perform offline unwinds by handling the creation of the
// `Regs`, `Maps`, and `Memory` objects needed for unwinding.
//
// `OfflineUnwindUtils` assists in two unwind use cases:
// 1. Single unwinds: unwind from a single sample/snapshot (one set of offline unwind files).
// 2. Consecutive/Multiple unwinds: unwind from a multiple samples/snapshots.
//
// `Init` contains two overloads for these two unwind cases. Other than `Init` and
// `ReturnToCurrentWorkingDirectory`, the remainder of the public API includes a `sample_name`
// parameter to indicate which sample/snapshot we are referencing. Specifying this value is
// REQUIRED for the multiple unwind use case. However, in the single use case, the caller has
// the choice of either providing the sample name or using the default value.
class OfflineUnwindUtils {
 public:
  // If the sample name passed to Get* is an invalid sample, nullptr is returned.
  Regs* GetRegs(const std::string& sample_name = kSingleSample) const;

  Maps* GetMaps(const std::string& sample_name = kSingleSample) const;

  std::shared_ptr<Memory> GetProcessMemory(const std::string& sample_name = kSingleSample) const;

  JitDebug* GetJitDebug(const std::string& sample_name = kSingleSample) const;

  const std::string* GetOfflineFilesPath(const std::string& sample_name = kSingleSample) const;

  const std::string* GetFrameInfoFilepath(const std::string& sample_name = kSingleSample) const;

  // Note: If the caller sets elements of `set_maps` to false or `memory_types` to
  //  kNoMemory, they are responsible for calling `CreateMaps` or `CreateProcessMemory` before
  //  expecting `GetMaps` or `GetProcessMemory` to return anything but nullptr.
  bool Init(const std::vector<UnwindSampleInfo>& sample_infos, std::string* error_msg);

  bool Init(const UnwindSampleInfo& sample_info, std::string* error_msg);

  // This must be called explicitly for the multiple unwind use case sometime before
  // Unwinder::Unwind is called. This is required because the Unwinder must init each
  // ELF object with a MemoryFileAtOffset memory object. Because the maps.txt provides a relative
  // path to the ELF files, we must be in the directory of the maps.txt when unwinding.
  //
  // Note: Init performs the check that this sample directory exists. If Init fails,
  // `initted_` is not set to true and this function will return false.
  bool ChangeToSampleDirectory(std::string* error_msg,
                               const std::string& initial_sample_name = kSingleSample) const;

  void ReturnToCurrentWorkingDirectory() {
    if (!cwd_.empty()) std::filesystem::current_path(cwd_);
  }

  bool GetExpectedNumFrames(size_t* expected_num_frames, std::string* error_msg,
                            const std::string& sample_name = kSingleSample) const;

  bool CreateMaps(std::string* error_msg, const std::string& sample_name = kSingleSample);

  bool CreateProcessMemory(std::string* error_msg, const std::string& sample_name = kSingleSample);

  static constexpr char kSingleSample[] = "";

 private:
  // An `UnwindSample` encapsulates the information necessary to perform an offline unwind for a
  // single offline sample/snapshot.
  struct UnwindSample {
    std::string offline_files_path;
    std::string frame_info_filepath;
    std::string map_buffer;
    std::unique_ptr<Regs> regs;
    std::unique_ptr<Maps> maps;
    std::shared_ptr<Memory> process_memory;
    std::unique_ptr<JitDebug> jit_debug;
  };

  bool CreateRegs(ArchEnum arch, std::string* error_msg,
                  const std::string& sample_name = kSingleSample);

  // Needed to support using the default value `kSingleSample` for the single unwind use case.
  const std::string& GetAdjustedSampleName(const std::string& sample_name) const;

  bool IsValidUnwindSample(const std::string& sample_name, std::string* error_msg) const;

  static std::unordered_map<std::string, uint32_t> arm_regs_;
  static std::unordered_map<std::string, uint32_t> arm64_regs_;
  static std::unordered_map<std::string, uint32_t> x86_regs_;
  static std::unordered_map<std::string, uint32_t> x86_64_regs_;

  std::string cwd_;
  std::unordered_map<std::string, UnwindSample> samples_;
  bool initted_ = false;
};

}  // namespace unwindstack
