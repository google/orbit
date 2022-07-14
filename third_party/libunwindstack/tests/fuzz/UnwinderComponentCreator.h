/*
 * Copyright 2020 The Android Open Source Project
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

#include <elf.h>
#include <sys/mman.h>

#include <memory>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>
#include <unwindstack/DexFiles.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Regs.h>
#include <unwindstack/RegsArm.h>
#include <unwindstack/RegsArm64.h>
#include <unwindstack/RegsMips.h>
#include <unwindstack/RegsMips64.h>
#include <unwindstack/RegsX86.h>
#include <unwindstack/RegsX86_64.h>

#include "../ElfFake.h"
#include "utils/MemoryFake.h"

#include "fuzzer/FuzzedDataProvider.h"

using unwindstack::ArchEnum;
using unwindstack::DexFiles;
using unwindstack::Elf;
using unwindstack::ElfFake;
using unwindstack::ElfInterfaceFake;
using unwindstack::FunctionData;
using unwindstack::Maps;
using unwindstack::Memory;
using unwindstack::MemoryFake;
using unwindstack::Regs;
using unwindstack::StepData;

static constexpr uint8_t kArchCount = 6;

static constexpr uint8_t kMaxSoNameLen = 150;

static constexpr uint8_t kMaxFuncNameLen = 50;
static constexpr uint8_t kMaxFuncCount = 100;

static constexpr uint8_t kMaxJitElfFiles = 20;
static constexpr uint8_t kJitElfPadding = 32;

static constexpr uint8_t kMaxStepCount = 100;
static constexpr uint8_t kMaxMapEntryCount = 50;
static constexpr uint8_t kMaxBuildIdLen = 100;
static constexpr uint8_t kMaxMapInfoNameLen = 150;

std::unique_ptr<unwindstack::Regs> GetRegisters(unwindstack::ArchEnum arch);
std::unique_ptr<unwindstack::Maps> GetMaps(FuzzedDataProvider* data_provider);
std::vector<std::string> GetStringList(FuzzedDataProvider* data_provider, uint max_str_len,
                                       uint max_strings);
unwindstack::ArchEnum GetArch(FuzzedDataProvider* data_provider);

void AddMapInfo(uint64_t start, uint64_t end, uint64_t offset, uint64_t flags, const char* name,
                Elf* elf = nullptr);
void PutElfFilesInMemory(MemoryFake* memory, FuzzedDataProvider* data_provider);

std::unique_ptr<unwindstack::DexFiles> GetDexFiles(FuzzedDataProvider* data_provider,
                                                   std::shared_ptr<unwindstack::Memory> memory,
                                                   uint max_libraries, uint max_library_length,
                                                   unwindstack::ArchEnum arch);
