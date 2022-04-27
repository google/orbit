/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <cstdio>
#define _GNU_SOURCE 1
#include <inttypes.h>
#include <stdio.h>
#include <sys/mman.h>

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <unwindstack/Elf.h>
#include <unwindstack/JitDebug.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Regs.h>
#include <unwindstack/Unwinder.h>
#include "utils/ProcessTracer.h"

#include <android-base/file.h>
#include <android-base/parseint.h>
#include <android-base/stringprintf.h>

namespace {
constexpr pid_t kMinPid = 1;
constexpr int kAllCmdOptionsParsed = -1;

struct map_info_t {
  uint64_t start;
  uint64_t end;
  uint64_t offset;
  uint64_t flags;
  std::string name;
};

int usage(int exit_code) {
  fprintf(stderr, "USAGE: unwind_for_offline [-t] [-e FILE] [-f[FILE]] <PID>\n\n");
  fprintf(stderr, "OPTIONS:\n");
  fprintf(stderr, "-t\n");
  fprintf(stderr, "       Dump offline snapshot for all threads of <PID>.\n");
  fprintf(stderr, "-e FILE\n");
  fprintf(stderr, "       If FILE is a valid ELF file included in /proc/<PID>/maps,\n");
  fprintf(stderr, "       unwind_for_offline will wait until the current frame (PC)\n");
  fprintf(stderr, "       lies within the .so file given by FILE. FILE should be\n");
  fprintf(stderr, "       base name of the path (the component following the final\n");
  fprintf(stderr, "       '/') rather than the fully qualified path.\n");
  fprintf(stderr, "-f [FILE]\n");
  fprintf(stderr, "       Write info (e.g. frames and stack range) logs to a file\n");
  fprintf(stderr, "       rather than to the stdout/stderr. If FILE is not\n");
  fprintf(stderr, "       specified, the output file will be named 'output.txt'.\n");
  return exit_code;
}

bool EnsureProcInDesiredElf(const std::string& elf_name, unwindstack::ProcessTracer& proc) {
  if (proc.UsesSharedLibrary(proc.pid(), elf_name)) {
    printf("Confirmed pid %d does use %s. Waiting for PC to lie within %s...\n", proc.pid(),
           elf_name.c_str(), elf_name.c_str());
    if (!proc.StopInDesiredElf(elf_name)) return false;
  } else {
    fprintf(stderr, "Process %d does not use library %s.\n", proc.pid(), elf_name.c_str());
    return false;
  }
  return true;
}

bool CreateAndChangeDumpDir(std::filesystem::path thread_dir, pid_t tid, bool is_main_thread) {
  std::string dir_name = std::to_string(tid);
  if (is_main_thread) dir_name += "_main-thread";
  thread_dir /= dir_name;
  if (!std::filesystem::create_directory(thread_dir)) {
    fprintf(stderr, "Failed to create directory for tid %d\n", tid);
    return false;
  }
  std::filesystem::current_path(thread_dir);
  return true;
}

bool SaveRegs(unwindstack::Regs* regs) {
  std::unique_ptr<FILE, decltype(&fclose)> fp(fopen("regs.txt", "w+"), &fclose);
  if (fp == nullptr) {
    perror("Failed to create file regs.txt");
    return false;
  }
  regs->IterateRegisters([&fp](const char* name, uint64_t value) {
    fprintf(fp.get(), "%s: %" PRIx64 "\n", name, value);
  });

  return true;
}

bool SaveStack(pid_t pid, const std::vector<std::pair<uint64_t, uint64_t>>& stacks,
               FILE* output_fp) {
  for (size_t i = 0; i < stacks.size(); i++) {
    std::string file_name;
    if (stacks.size() != 1) {
      file_name = "stack" + std::to_string(i) + ".data";
    } else {
      file_name = "stack.data";
    }

    // Do this first, so if it fails, we don't create the file.
    uint64_t sp_start = stacks[i].first;
    uint64_t sp_end = stacks[i].second;
    std::vector<uint8_t> buffer(sp_end - sp_start);
    auto process_memory = unwindstack::Memory::CreateProcessMemory(pid);
    if (!process_memory->Read(sp_start, buffer.data(), buffer.size())) {
      fprintf(stderr, "Unable to read stack data.\n");
      return false;
    }

    fprintf(output_fp, "\nSaving the stack 0x%" PRIx64 "-0x%" PRIx64 "\n", sp_start, sp_end);

    std::unique_ptr<FILE, decltype(&fclose)> fp(fopen(file_name.c_str(), "w+"), &fclose);
    if (fp == nullptr) {
      perror("Failed to create stack.data");
      return false;
    }

    size_t bytes = fwrite(&sp_start, 1, sizeof(sp_start), fp.get());
    if (bytes != sizeof(sp_start)) {
      fprintf(stderr, "Failed to write sp_start data: sizeof(sp_start) %zu, written %zu\n",
              sizeof(sp_start), bytes);
      return false;
    }

    bytes = fwrite(buffer.data(), 1, buffer.size(), fp.get());
    if (bytes != buffer.size()) {
      fprintf(stderr, "Failed to write all stack data: stack size %zu, written %zu\n",
              buffer.size(), bytes);
      return false;
    }
  }

  return true;
}

bool CreateElfFromMemory(std::shared_ptr<unwindstack::Memory>& memory, map_info_t* info) {
  std::string cur_name;
  if (info->name.empty()) {
    cur_name = android::base::StringPrintf("anonymous_%" PRIx64, info->start);
  } else {
    cur_name = android::base::StringPrintf(
        "%s_%" PRIx64, android::base::Basename(info->name).c_str(), info->start);
  }

  std::vector<uint8_t> buffer(info->end - info->start);
  // If this is a mapped in file, it might not be possible to read the entire
  // map, so read all that is readable.
  size_t bytes = memory->Read(info->start, buffer.data(), buffer.size());
  if (bytes == 0) {
    fprintf(stderr, "Cannot read data from address %" PRIx64 " length %zu\n", info->start,
            buffer.size());
    return false;
  }

  std::unique_ptr<FILE, decltype(&fclose)> output(fopen(cur_name.c_str(), "w+"), &fclose);
  if (output == nullptr) {
    perror((std::string("Cannot create ") + cur_name).c_str());
    return false;
  }

  size_t bytes_written = fwrite(buffer.data(), 1, bytes, output.get());
  if (bytes_written != bytes) {
    fprintf(stderr, "Failed to write all data to file: bytes read %zu, written %zu\n", bytes,
            bytes_written);
    return false;
  }

  // Replace the name with the new name.
  info->name = cur_name;

  return true;
}

bool CopyElfFromFile(map_info_t* info, bool* file_copied) {
  std::string cur_name = android::base::Basename(info->name);
  if (*file_copied) {
    info->name = cur_name;
    return true;
  }

  std::unique_ptr<FILE, decltype(&fclose)> fp(fopen(info->name.c_str(), "r"), &fclose);
  if (fp == nullptr) {
    perror((std::string("Cannot open ") + info->name).c_str());
    return false;
  }

  std::unique_ptr<FILE, decltype(&fclose)> output(fopen(cur_name.c_str(), "w+"), &fclose);
  if (output == nullptr) {
    perror((std::string("Cannot create file " + cur_name)).c_str());
    return false;
  }
  std::vector<uint8_t> buffer(10000);
  size_t bytes;
  while ((bytes = fread(buffer.data(), 1, buffer.size(), fp.get())) > 0) {
    size_t bytes_written = fwrite(buffer.data(), 1, bytes, output.get());
    if (bytes_written != bytes) {
      fprintf(stderr, "Bytes written doesn't match bytes read: read %zu, written %zu\n", bytes,
              bytes_written);
      return false;
    }
  }

  // Replace the name with the new name.
  info->name = cur_name;

  return true;
}

map_info_t* FillInAndGetMapInfo(std::unordered_map<uint64_t, map_info_t>& maps_by_start,
                                unwindstack::MapInfo* map_info) {
  auto info = &maps_by_start[map_info->start()];
  info->start = map_info->start();
  info->end = map_info->end();
  info->offset = map_info->offset();
  info->name = map_info->name();
  info->flags = map_info->flags();

  return info;
}

void SaveMapInformation(std::shared_ptr<unwindstack::Memory>& process_memory, map_info_t* info,
                        bool* file_copied) {
  if (CopyElfFromFile(info, file_copied)) {
    return;
  }
  *file_copied = false;

  // Try to create the elf from memory, this will handle cases where
  // the data only exists in memory such as vdso data on x86.
  if (CreateElfFromMemory(process_memory, info)) {
    return;
  }

  fprintf(stderr, "Cannot save memory or file for map ");
  if (!info->name.empty()) {
    fprintf(stderr, "%s\n", info->name.c_str());
  } else {
    fprintf(stderr, "anonymous:%" PRIx64 "\n", info->start);
  }
}

bool SaveData(pid_t tid, const std::filesystem::path& cwd, bool is_main_thread, FILE* output_fp) {
  fprintf(output_fp, "-------------------- tid = %d %s--------------------\n", tid,
          is_main_thread ? "(main thread) " : "--------------");
  unwindstack::Regs* regs = unwindstack::Regs::RemoteGet(tid);
  if (regs == nullptr) {
    fprintf(stderr, "Unable to get remote reg data.\n");
    return false;
  }

  if (!CreateAndChangeDumpDir(cwd, tid, is_main_thread)) return false;

  // Save the current state of the registers.
  if (!SaveRegs(regs)) {
    return false;
  }

  // Do an unwind so we know how much of the stack to save, and what
  // elf files are involved.
  unwindstack::UnwinderFromPid unwinder(1024, tid);
  unwinder.SetRegs(regs);
  uint64_t sp = regs->sp();
  unwinder.Unwind();

  std::unordered_map<uint64_t, map_info_t> maps_by_start;
  std::vector<std::pair<uint64_t, uint64_t>> stacks;
  unwindstack::Maps* maps = unwinder.GetMaps();
  uint64_t sp_map_start = 0;
  auto map_info = maps->Find(sp);
  if (map_info != nullptr) {
    stacks.emplace_back(std::make_pair(sp, map_info->end()));
    sp_map_start = map_info->start();
  }

  for (const auto& frame : unwinder.frames()) {
    map_info = maps->Find(frame.sp);
    if (map_info != nullptr && sp_map_start != map_info->start()) {
      stacks.emplace_back(std::make_pair(frame.sp, map_info->end()));
      sp_map_start = map_info->start();
    }

    if (maps_by_start.count(frame.map_start) == 0) {
      map_info = maps->Find(frame.map_start);
      if (map_info == nullptr) {
        continue;
      }

      auto info = FillInAndGetMapInfo(maps_by_start, map_info.get());
      bool file_copied = false;
      SaveMapInformation(unwinder.GetProcessMemory(), info, &file_copied);

      // If you are using a a linker that creates two maps (one read-only, one
      // read-executable), it's necessary to capture the previous map
      // information if needed.
      auto prev_map = map_info->prev_map();
      if (prev_map != nullptr && map_info->offset() != 0 && prev_map->offset() == 0 &&
          prev_map->flags() == PROT_READ && map_info->name() == prev_map->name() &&
          maps_by_start.count(prev_map->start()) == 0) {
        info = FillInAndGetMapInfo(maps_by_start, prev_map.get());
        SaveMapInformation(unwinder.GetProcessMemory(), info, &file_copied);
      }
    }
  }

  for (size_t i = 0; i < unwinder.NumFrames(); i++) {
    fprintf(output_fp, "%s\n", unwinder.FormatFrame(i).c_str());
  }

  if (!SaveStack(tid, stacks, output_fp)) {
    return false;
  }

  std::vector<std::pair<uint64_t, map_info_t>> sorted_maps(maps_by_start.begin(),
                                                           maps_by_start.end());
  std::sort(sorted_maps.begin(), sorted_maps.end(),
            [](auto& a, auto& b) { return a.first < b.first; });

  std::unique_ptr<FILE, decltype(&fclose)> map_fp(fopen("maps.txt", "w+"), &fclose);
  if (map_fp == nullptr) {
    perror("Failed to create maps.txt");
    return false;
  }

  for (auto& element : sorted_maps) {
    char perms[5] = {"---p"};
    map_info_t& map = element.second;
    if (map.flags & PROT_READ) {
      perms[0] = 'r';
    }
    if (map.flags & PROT_WRITE) {
      perms[1] = 'w';
    }
    if (map.flags & PROT_EXEC) {
      perms[2] = 'x';
    }
    fprintf(map_fp.get(), "%" PRIx64 "-%" PRIx64 " %s %" PRIx64 " 00:00 0", map.start, map.end,
            perms, map.offset);
    if (!map.name.empty()) {
      fprintf(map_fp.get(), "   %s", map.name.c_str());
    }
    fprintf(map_fp.get(), "\n");
  }

  fprintf(output_fp, "------------------------------------------------------------------\n");
  return true;
}
}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) return usage(EXIT_FAILURE);

  bool dump_threads = false;
  std::string elf_name;
  std::unique_ptr<FILE, decltype(&fclose)> output_fp(nullptr, &fclose);
  int opt;
  while ((opt = getopt(argc, argv, ":te:f::")) != kAllCmdOptionsParsed) {
    switch (opt) {
      case 't': {
        dump_threads = true;
        break;
      }
      case 'e': {
        elf_name = optarg;
        if (elf_name == "-f") {
          fprintf(stderr, "Missing argument for option e.\n");
          return usage(EXIT_FAILURE);
        }
        break;
      }
      case 'f': {
        const std::string& output_filename = optarg != nullptr ? optarg : "output.txt";
        if (optind == argc - 2) {
          fprintf(stderr, "Ensure there is no space between '-f' and the filename provided.\n");
          return usage(EXIT_FAILURE);
        }
        output_fp.reset(fopen(output_filename.c_str(), "a"));
        break;
      }
      case '?': {
        if (isprint(optopt))
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
        return usage(EXIT_FAILURE);
      }
      case ':': {
        fprintf(stderr, "Missing arg for option %c.\n", optopt);
        return usage(EXIT_FAILURE);
      }
      default: {
        return usage(EXIT_FAILURE);
      }
    }
  }
  if (optind != argc - 1) return usage(EXIT_FAILURE);

  pid_t pid;
  if (!android::base::ParseInt(argv[optind], &pid, kMinPid, std::numeric_limits<pid_t>::max()))
    return usage(EXIT_FAILURE);

  unwindstack::ProcessTracer proc(pid, dump_threads);
  if (!proc.Stop()) return EXIT_FAILURE;
  if (!elf_name.empty()) {
    if (!EnsureProcInDesiredElf(elf_name, proc)) return EXIT_FAILURE;
  }
  if (!output_fp) output_fp.reset(stdout);
  std::filesystem::path cwd = std::filesystem::current_path();

  if (!proc.Attach(proc.pid())) return EXIT_FAILURE;
  if (!SaveData(proc.pid(), cwd, /*is_main_thread=*/proc.IsTracingThreads(), output_fp.get()))
    return EXIT_FAILURE;
  if (!proc.Detach(proc.pid())) return EXIT_FAILURE;
  for (const pid_t& tid : proc.tids()) {
    if (!proc.Attach(tid)) return EXIT_FAILURE;
    if (!SaveData(tid, cwd, /*is_main_thread=*/false, output_fp.get())) return EXIT_FAILURE;
    if (!proc.Detach(tid)) return EXIT_FAILURE;
  }

  printf("\nSuccess!\n");
  return EXIT_SUCCESS;
}
