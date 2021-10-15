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

#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <android-base/file.h>
#include <android-base/strings.h>
#include <zlib.h>

#include <unwindstack/Arch.h>
#include <unwindstack/JitDebug.h>
#include <unwindstack/MachineArm.h>
#include <unwindstack/MachineArm64.h>
#include <unwindstack/MachineX86.h>
#include <unwindstack/MachineX86_64.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Regs.h>
#include <unwindstack/RegsArm.h>
#include <unwindstack/RegsArm64.h>
#include <unwindstack/RegsX86.h>
#include <unwindstack/RegsX86_64.h>
#include <unwindstack/Unwinder.h>

#include "Check.h"
#include "MemoryOffline.h"
#include "utils/MemoryFake.h"

#include "OfflineUnwindUtils.h"

namespace unwindstack {

void DecompressFiles(const std::string& directory) {
  namespace fs = std::filesystem;
  for (const auto& file : fs::recursive_directory_iterator(directory)) {
    fs::path src_path = file.path();
    if (src_path.extension() == ".gz") {
      fs::path dst_path = fs::path(src_path).replace_extension();  // Remove .gz extension.
      if (!fs::exists(dst_path) || fs::last_write_time(src_path) > fs::last_write_time(dst_path)) {
        gzFile src = gzopen(src_path.c_str(), "rb");
        CHECK(src != nullptr);
        fs::path tmp_path = fs::path(src_path).replace_extension("." + std::to_string(getpid()));
        std::ofstream tmp(tmp_path);  // Temporary file to avoid races between unit tests.
        char buffer[1024];
        int size;
        while ((size = gzread(src, buffer, sizeof(buffer))) > 0) {
          tmp.write(buffer, size);
        }
        tmp.close();
        gzclose(src);
        fs::rename(tmp_path, dst_path);
      }
    }
  }
}

void CreateLinks(const std::string& directory) {
  namespace fs = std::filesystem;
  for (const auto& file : fs::recursive_directory_iterator(directory)) {
    fs::path src_path = file.path();
    if (fs::is_regular_file(src_path) && src_path.filename() == "links.txt") {
      std::string contents;
      if (!android::base::ReadFileToString(src_path.c_str(), &contents)) {
        errx(1, "Unable to read file: %s", src_path.c_str());
      }
      fs::path parent_path = src_path.parent_path();
      std::vector<std::string> lines(android::base::Split(contents, "\n"));
      for (auto line : lines) {
        std::string trimmed_line(android::base::Trim(line));
        if (trimmed_line.empty()) {
          continue;
        }

        std::vector<std::string> values(android::base::Split(trimmed_line, " "));
        if (values.size() != 2) {
          errx(1, "Invalid line in %s: line %s", src_path.c_str(), line.c_str());
        }

        // Create the symlink if it doesn't already exist.
        fs::path target(parent_path);
        target /= fs::path(values[0]);
        fs::path source(parent_path);
        source /= fs::path(values[1]);
        if (!fs::exists(source)) {
          // Ignore any errors, if this is running at the same time
          // in multiple processes, then this might fail.
          std::error_code ec;
          fs::create_symlink(target, source, ec);
        }
      }
    }
  }
}

std::string GetOfflineFilesDirectory() {
  std::string path = android::base::GetExecutableDirectory() + "/offline_files/";
  DecompressFiles(path);
  CreateLinks(path);
  return path;
}

std::string DumpFrames(const Unwinder& unwinder) {
  std::string str;
  for (size_t i = 0; i < unwinder.NumFrames(); i++) {
    str += unwinder.FormatFrame(i) + "\n";
  }
  return str;
}

bool AddMemory(std::string file_name, MemoryOfflineParts* parts, std::string* error_msg) {
  MemoryOffline* memory = new MemoryOffline;
  if (!memory->Init(file_name.c_str(), 0)) {
    std::stringstream err_stream;
    err_stream << "Failed to add stack '" << file_name << "' to stack memory.";
    *error_msg = err_stream.str();
    return false;
  }
  parts->Add(memory);

  return true;
}

Regs* OfflineUnwindUtils::GetRegs(const std::string& initial_sample_name) const {
  const std::string& sample_name = GetAdjustedSampleName(initial_sample_name);
  std::string error_msg;
  if (!IsValidUnwindSample(sample_name, &error_msg)) {
    std::cerr << error_msg;
    return nullptr;
  }
  return samples_.at(sample_name).regs.get();
}

Maps* OfflineUnwindUtils::GetMaps(const std::string& initial_sample_name) const {
  const std::string& sample_name = GetAdjustedSampleName(initial_sample_name);
  std::string error_msg;
  if (!IsValidUnwindSample(sample_name, &error_msg)) {
    std::cerr << error_msg;
    return nullptr;
  }
  return samples_.at(sample_name).maps.get();
}

std::shared_ptr<Memory> OfflineUnwindUtils::GetProcessMemory(
    const std::string& initial_sample_name) const {
  const std::string& sample_name = GetAdjustedSampleName(initial_sample_name);
  std::string error_msg;
  if (!IsValidUnwindSample(sample_name, &error_msg)) {
    std::cerr << error_msg;
    return nullptr;
  }
  return samples_.at(sample_name).process_memory;
}

JitDebug* OfflineUnwindUtils::GetJitDebug(const std::string& initial_sample_name) const {
  const std::string& sample_name = GetAdjustedSampleName(initial_sample_name);
  std::string error_msg;
  if (!IsValidUnwindSample(sample_name, &error_msg)) {
    std::cerr << error_msg;
    return nullptr;
  }
  return samples_.at(sample_name).jit_debug.get();
}

const std::string* OfflineUnwindUtils::GetOfflineFilesPath(
    const std::string& initial_sample_name) const {
  const std::string& sample_name = GetAdjustedSampleName(initial_sample_name);
  std::string error_msg;
  if (!IsValidUnwindSample(sample_name, &error_msg)) {
    std::cerr << error_msg;
    return nullptr;
  }
  return &samples_.at(sample_name).offline_files_path;
}

const std::string* OfflineUnwindUtils::GetFrameInfoFilepath(
    const std::string& initial_sample_name) const {
  const std::string& sample_name = GetAdjustedSampleName(initial_sample_name);
  std::string error_msg;
  if (!IsValidUnwindSample(sample_name, &error_msg)) {
    std::cerr << error_msg;
    return nullptr;
  }
  return &samples_.at(sample_name).frame_info_filepath;
}

bool OfflineUnwindUtils::Init(const std::vector<UnwindSampleInfo>& sample_infos,
                              std::string* error_msg) {
  // Save the current path so the caller can switch back to it later.
  cwd_ = std::filesystem::current_path();

  // Fill in the unwind samples.
  std::stringstream err_stream;
  for (const auto& sample_info : sample_infos) {
    std::string offline_files_full_path =
        GetOfflineFilesDirectory() + sample_info.offline_files_dir;
    if (!std::filesystem::exists(offline_files_full_path)) {
      err_stream << "Offline files directory '" << offline_files_full_path << "' does not exist.";
      *error_msg = err_stream.str();
      return false;
    }
    std::string frame_info_filepath = offline_files_full_path + sample_info.frame_info_filename;

    std::string map_buffer;
    if (!android::base::ReadFileToString((offline_files_full_path + "maps.txt"), &map_buffer)) {
      err_stream << "Failed to read from '" << offline_files_full_path << "maps.txt' into memory.";
      *error_msg = err_stream.str();
      return false;
    }

    // CreateMaps, CreatRegs, and Create*Memory may need to be called later by the client. So we
    // need to create the sample now in case the flags are set to call these methods in Init.
    const std::string& sample_name = sample_info.offline_files_dir;
    samples_.emplace(sample_name, (UnwindSample){
                                      std::move(offline_files_full_path),
                                      std::move(frame_info_filepath), std::move(map_buffer),
                                      nullptr,                         // regs
                                      nullptr,                         // maps
                                      std::make_shared<MemoryFake>(),  // process_memory
                                      nullptr,                         // jit_debug
                                  });
    UnwindSample& sample = samples_.at(sample_name);

    if (sample_info.create_maps) {
      if (!CreateMaps(error_msg, sample_name)) return false;
    }
    if (!CreateRegs(sample_info.arch, error_msg, sample_name)) return false;

    switch (sample_info.memory_flag) {
      case ProcessMemoryFlag::kNone: {
        if (!CreateProcessMemory(error_msg, sample_name)) return false;
        break;
      }
      case ProcessMemoryFlag::kIncludeJitMemory: {
        if (!CreateProcessMemory(error_msg, sample_name)) return false;
        sample.jit_debug = CreateJitDebug(sample.regs->Arch(), sample.process_memory);
        break;
      }
      case ProcessMemoryFlag::kNoMemory: {
        break;
      }
      default: {
        std::stringstream err_stream;
        err_stream << "Unknown memory type for sample '" << sample_name << "'.";
        *error_msg = err_stream.str();
        return false;
      }
    }
  }
  initted_ = true;
  return true;
}

bool OfflineUnwindUtils::Init(const UnwindSampleInfo& sample_info, std::string* error_msg) {
  if (Init(std::vector<UnwindSampleInfo>{sample_info}, error_msg)) {
    if (!ChangeToSampleDirectory(error_msg)) return false;
    return true;
  }
  return false;
}

bool OfflineUnwindUtils::ChangeToSampleDirectory(std::string* error_msg,
                                                 const std::string& initial_sample_name) const {
  if (!initted_) {
    *error_msg =
        "Cannot change to sample directory because OfflineUnwindUtils::Init has not been called.";
    return false;
  }
  const std::string& sample_name = GetAdjustedSampleName(initial_sample_name);
  if (!IsValidUnwindSample(sample_name, error_msg)) return false;

  std::filesystem::current_path(std::filesystem::path(samples_.at(sample_name).offline_files_path));
  return true;
}

bool OfflineUnwindUtils::GetExpectedNumFrames(size_t* expected_num_frames, std::string* error_msg,
                                              const std::string& initial_sample_name) const {
  if (!initted_) {
    *error_msg =
        "Cannot get expected number of frames of a sample because OfflineUnwindUtils::Init has not "
        "been called.";
    return false;
  }
  const std::string& sample_name = GetAdjustedSampleName(initial_sample_name);
  if (!IsValidUnwindSample(sample_name, error_msg)) return false;

  const std::string& sample_frames_path = samples_.at(sample_name).frame_info_filepath;
  if (!std::filesystem::exists(sample_frames_path)) {
    std::stringstream err_stream;
    err_stream << "Offline files directory '" << sample_frames_path << "' does not exist.";
    *error_msg = err_stream.str();
    return false;
  }

  std::ifstream in(sample_frames_path);
  in.unsetf(std::ios_base::skipws);  // Ensure that we do not skip newlines.
  *expected_num_frames =
      std::count(std::istream_iterator<char>(in), std::istream_iterator<char>(), '\n');

  return true;
}

bool OfflineUnwindUtils::CreateMaps(std::string* error_msg,
                                    const std::string& initial_sample_name) {
  const std::string& sample_name = GetAdjustedSampleName(initial_sample_name);
  if (!IsValidUnwindSample(sample_name, error_msg)) return false;
  UnwindSample& sample = samples_.at(sample_name);

  sample.maps.reset(new BufferMaps(sample.map_buffer.c_str()));
  if (!sample.maps->Parse()) {
    *error_msg = "Failed to parse offline maps.";
    return false;
  }
  return true;
}

bool OfflineUnwindUtils::CreateProcessMemory(std::string* error_msg,
                                             const std::string& initial_sample_name) {
  const std::string& sample_name = GetAdjustedSampleName(initial_sample_name);
  if (!IsValidUnwindSample(sample_name, error_msg)) return false;
  UnwindSample& sample = samples_.at(sample_name);

  // Construct process memory from all descriptor, stack, entry, and jit files
  auto memory = std::make_unique<MemoryOfflineParts>();
  bool data_files_found = false;
  for (const auto& file : std::filesystem::directory_iterator(sample.offline_files_path)) {
    std::string filename = file.path().string();
    if (std::regex_match(filename,
                         std::regex("^(.+)\\/(descriptor|stack|entry|jit)(\\d*)\\.data$"))) {
      data_files_found = true;
      if (!AddMemory(filename, memory.get(), error_msg)) return false;
    }
  }
  if (!data_files_found) {
    *error_msg = "No memory (stack, JIT, etc.) data files found.";
    return false;
  }

  sample.process_memory.reset(memory.release());
  return true;
}

namespace {
template <typename AddressType>
bool ReadRegs(RegsImpl<AddressType>* regs,
              const std::unordered_map<std::string, uint32_t>& name_to_reg, std::string* error_msg,
              const std::string& offline_files_path) {
  std::stringstream err_stream;
  FILE* fp = fopen((offline_files_path + "regs.txt").c_str(), "r");
  if (fp == nullptr) {
    err_stream << "Error opening file '" << offline_files_path << "regs.txt': " << strerror(errno);
    *error_msg = err_stream.str();
    return false;
  }

  while (!feof(fp)) {
    uint64_t value;
    char reg_name[100];
    if (fscanf(fp, "%s %" SCNx64 "\n", reg_name, &value) != 2) {
      err_stream << "Failed to read in register name/values from '" << offline_files_path
                 << "regs.txt'.";
      *error_msg = err_stream.str();
      return false;
    }
    std::string name(reg_name);
    if (!name.empty()) {
      // Remove the : from the end.
      name.resize(name.size() - 1);
    }
    auto entry = name_to_reg.find(name);
    if (entry == name_to_reg.end()) {
      err_stream << "Unknown register named " << reg_name;
      *error_msg = err_stream.str();
      return false;
    }
    (*regs)[entry->second] = value;
  }
  fclose(fp);
  return true;
}
}  // namespace

bool OfflineUnwindUtils::CreateRegs(ArchEnum arch, std::string* error_msg,
                                    const std::string& initial_sample_name) {
  const std::string& sample_name = GetAdjustedSampleName(initial_sample_name);
  if (!IsValidUnwindSample(sample_name, error_msg)) return false;
  auto& regs = samples_.at(sample_name).regs;
  const auto& offline_files_path = samples_.at(sample_name).offline_files_path;

  switch (arch) {
    case ARCH_ARM: {
      RegsArm* regs_impl = new RegsArm;
      regs.reset(regs_impl);
      if (!ReadRegs<uint32_t>(regs_impl, arm_regs_, error_msg, offline_files_path)) return false;
      break;
    }
    case ARCH_ARM64: {
      RegsArm64* regs_impl = new RegsArm64;
      regs.reset(regs_impl);
      if (!ReadRegs<uint64_t>(regs_impl, arm64_regs_, error_msg, offline_files_path)) return false;
      break;
    }
    case ARCH_X86: {
      RegsX86* regs_impl = new RegsX86;
      regs.reset(regs_impl);
      if (!ReadRegs<uint32_t>(regs_impl, x86_regs_, error_msg, offline_files_path)) return false;
      break;
    }
    case ARCH_X86_64: {
      RegsX86_64* regs_impl = new RegsX86_64;
      regs.reset(regs_impl);
      if (!ReadRegs<uint64_t>(regs_impl, x86_64_regs_, error_msg, offline_files_path)) return false;
      break;
    }
    default:
      *error_msg = "Unknown architechture " + std::to_string(arch);
      return false;
  }

  return true;
}

const std::string& OfflineUnwindUtils::GetAdjustedSampleName(
    const std::string& initial_sample_name) const {
  // Only return the first entry in the sample map if this is the single unwind use case.
  // Otherwise return the inputted sample name so we can check if that is a valid sample name.
  if (initial_sample_name == kSingleSample && samples_.size() == 1) {
    return samples_.begin()->first;
  }
  return initial_sample_name;
}

bool OfflineUnwindUtils::IsValidUnwindSample(const std::string& sample_name,
                                             std::string* error_msg) const {
  if (samples_.find(sample_name) == samples_.end()) {
    std::stringstream err_stream;
    err_stream << "Invalid sample name (offline file directory) '" << sample_name << "'.";
    if (sample_name == kSingleSample) {
      err_stream << " An explicit sample name must be provided for the multiple unwind use case "
                    "of OfflineUnwindUtils (i.e. should not use the default sample name).";
    }
    *error_msg = err_stream.str();
    return false;
  }
  return true;
}

std::unordered_map<std::string, uint32_t> OfflineUnwindUtils::arm_regs_ = {
    {"r0", ARM_REG_R0},  {"r1", ARM_REG_R1}, {"r2", ARM_REG_R2},   {"r3", ARM_REG_R3},
    {"r4", ARM_REG_R4},  {"r5", ARM_REG_R5}, {"r6", ARM_REG_R6},   {"r7", ARM_REG_R7},
    {"r8", ARM_REG_R8},  {"r9", ARM_REG_R9}, {"r10", ARM_REG_R10}, {"r11", ARM_REG_R11},
    {"ip", ARM_REG_R12}, {"sp", ARM_REG_SP}, {"lr", ARM_REG_LR},   {"pc", ARM_REG_PC},
};

std::unordered_map<std::string, uint32_t> OfflineUnwindUtils::arm64_regs_ = {
    {"x0", ARM64_REG_R0},      {"x1", ARM64_REG_R1},   {"x2", ARM64_REG_R2},
    {"x3", ARM64_REG_R3},      {"x4", ARM64_REG_R4},   {"x5", ARM64_REG_R5},
    {"x6", ARM64_REG_R6},      {"x7", ARM64_REG_R7},   {"x8", ARM64_REG_R8},
    {"x9", ARM64_REG_R9},      {"x10", ARM64_REG_R10}, {"x11", ARM64_REG_R11},
    {"x12", ARM64_REG_R12},    {"x13", ARM64_REG_R13}, {"x14", ARM64_REG_R14},
    {"x15", ARM64_REG_R15},    {"x16", ARM64_REG_R16}, {"x17", ARM64_REG_R17},
    {"x18", ARM64_REG_R18},    {"x19", ARM64_REG_R19}, {"x20", ARM64_REG_R20},
    {"x21", ARM64_REG_R21},    {"x22", ARM64_REG_R22}, {"x23", ARM64_REG_R23},
    {"x24", ARM64_REG_R24},    {"x25", ARM64_REG_R25}, {"x26", ARM64_REG_R26},
    {"x27", ARM64_REG_R27},    {"x28", ARM64_REG_R28}, {"x29", ARM64_REG_R29},
    {"sp", ARM64_REG_SP},      {"lr", ARM64_REG_LR},   {"pc", ARM64_REG_PC},
    {"pst", ARM64_REG_PSTATE},
};

std::unordered_map<std::string, uint32_t> OfflineUnwindUtils::x86_regs_ = {
    {"eax", X86_REG_EAX}, {"ebx", X86_REG_EBX}, {"ecx", X86_REG_ECX},
    {"edx", X86_REG_EDX}, {"ebp", X86_REG_EBP}, {"edi", X86_REG_EDI},
    {"esi", X86_REG_ESI}, {"esp", X86_REG_ESP}, {"eip", X86_REG_EIP},
};

std::unordered_map<std::string, uint32_t> OfflineUnwindUtils::x86_64_regs_ = {
    {"rax", X86_64_REG_RAX}, {"rbx", X86_64_REG_RBX}, {"rcx", X86_64_REG_RCX},
    {"rdx", X86_64_REG_RDX}, {"r8", X86_64_REG_R8},   {"r9", X86_64_REG_R9},
    {"r10", X86_64_REG_R10}, {"r11", X86_64_REG_R11}, {"r12", X86_64_REG_R12},
    {"r13", X86_64_REG_R13}, {"r14", X86_64_REG_R14}, {"r15", X86_64_REG_R15},
    {"rdi", X86_64_REG_RDI}, {"rsi", X86_64_REG_RSI}, {"rbp", X86_64_REG_RBP},
    {"rsp", X86_64_REG_RSP}, {"rip", X86_64_REG_RIP},
};

}  // namespace unwindstack
