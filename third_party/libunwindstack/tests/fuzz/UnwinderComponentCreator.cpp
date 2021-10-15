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

#include "UnwinderComponentCreator.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

std::unique_ptr<Regs> GetRegisters(ArchEnum arch) {
  switch (arch) {
    case unwindstack::ARCH_ARM: {
      std::unique_ptr<unwindstack::RegsArm> regs = std::make_unique<unwindstack::RegsArm>();
      return regs;
    }
    case unwindstack::ARCH_ARM64: {
      std::unique_ptr<unwindstack::RegsArm64> regs = std::make_unique<unwindstack::RegsArm64>();
      return regs;
    }
    case unwindstack::ARCH_X86: {
      std::unique_ptr<unwindstack::RegsX86> regs = std::make_unique<unwindstack::RegsX86>();
      return regs;
    }
    case unwindstack::ARCH_X86_64: {
      std::unique_ptr<unwindstack::RegsX86_64> regs = std::make_unique<unwindstack::RegsX86_64>();
      return regs;
    }
    case unwindstack::ARCH_MIPS: {
      std::unique_ptr<unwindstack::RegsMips> regs = std::make_unique<unwindstack::RegsMips>();
      return regs;
    }
    case unwindstack::ARCH_MIPS64: {
      std::unique_ptr<unwindstack::RegsMips64> regs = std::make_unique<unwindstack::RegsMips64>();
      return regs;
    }
    case unwindstack::ARCH_UNKNOWN:
    default: {
      std::unique_ptr<unwindstack::RegsX86_64> regs = std::make_unique<unwindstack::RegsX86_64>();
      return regs;
    }
  }
}

ArchEnum GetArch(FuzzedDataProvider* data_provider) {
  uint8_t arch = data_provider->ConsumeIntegralInRange<uint8_t>(1, kArchCount);
  return static_cast<ArchEnum>(arch);
}

void ElfAddMapInfo(Maps* maps, uint64_t start, uint64_t end, uint64_t offset, uint64_t flags,
                   const char* name, Elf* elf = nullptr) {
  std::string str_name(name);
  maps->Add(start, end, offset, flags, name, static_cast<uint64_t>(-1));
  if (elf != nullptr) {
    const auto& map_info = *--maps->end();
    map_info->set_elf(elf);
  }
}

void ElfPushFakeFunctionData(FuzzedDataProvider* data_provider, ElfInterfaceFake* elf) {
  uint8_t func_count = data_provider->ConsumeIntegralInRange<uint>(0, kMaxFuncCount);
  for (uint8_t i = 0; i < func_count; i++) {
    std::string func_name = data_provider->ConsumeRandomLengthString(kMaxFuncNameLen);
    bool global = data_provider->ConsumeBool();
    if (global) {
      elf->FakeSetGlobalVariable(func_name, data_provider->ConsumeIntegral<uint64_t>());
    } else {
      ElfInterfaceFake::FakePushFunctionData(FunctionData(func_name, i));
    }
  }
}
void ElfPushFakeStepData(FuzzedDataProvider* data_provider) {
  uint8_t step_count = data_provider->ConsumeIntegralInRange<uint>(0, kMaxStepCount);
  for (uint8_t i = 0; i < step_count; i++) {
    uint64_t pc = data_provider->ConsumeIntegral<uint64_t>();
    uint64_t sp = data_provider->ConsumeIntegral<uint64_t>();
    bool finished = i + 1 == step_count;
    ElfInterfaceFake::FakePushStepData(StepData(pc, sp, finished));
  }
}

ElfFake* PopulateElfFake(FuzzedDataProvider* data_provider) {
  // This will be passed to a smart pointer in ElfAddMapInfo.
  ElfFake* elf = new ElfFake(new MemoryFake);

  // This will be handled by a smart pointer within Elf.
  ElfInterfaceFake* interface_fake = new ElfInterfaceFake(nullptr);
  std::string build_id = data_provider->ConsumeRandomLengthString(kMaxBuildIdLen);
  interface_fake->FakeSetBuildID(build_id);
  std::string so_name = data_provider->ConsumeRandomLengthString(kMaxSoNameLen);
  interface_fake->FakeSetSoname(so_name.c_str());

  elf->FakeSetArch(GetArch(data_provider));
  elf->FakeSetLoadBias(data_provider->ConsumeIntegral<uint64_t>());

  ElfPushFakeFunctionData(data_provider, interface_fake);
  ElfPushFakeStepData(data_provider);

  elf->FakeSetInterface(interface_fake);
  ElfInterfaceFake::FakeClear();
  return elf;
}

static constexpr size_t kPageSize = 4096;

static inline bool AlignToPage(uint64_t address, uint64_t* aligned_address) {
  if (__builtin_add_overflow(address, kPageSize - 1, aligned_address)) {
    return false;
  }
  *aligned_address &= ~(kPageSize - 1);
  return true;
}

std::unique_ptr<Maps> GetMaps(FuzzedDataProvider* data_provider) {
  std::unique_ptr<Maps> maps = std::make_unique<Maps>();
  std::map<uint64_t, uint64_t> map_ends;
  uint8_t entry_count = data_provider->ConsumeIntegralInRange<uint8_t>(0, kMaxMapEntryCount);
  for (uint8_t i = 0; i < entry_count; i++) {
    uint64_t start;
    if (!AlignToPage(data_provider->ConsumeIntegral<uint64_t>(), &start)) {
      // Overflowed.
      continue;
    }
    uint64_t end;
    if (!AlignToPage(data_provider->ConsumeIntegralInRange<uint64_t>(start, UINT64_MAX), &end)) {
      // Overflowed.
      continue;
    }
    if (start == end) {
      // It's impossible to see start == end in the real world, so
      // make sure the map contains at least one page of data.
      if (__builtin_add_overflow(end, 0x1000, &end)) {
        continue;
      }
    }
    // Make sure not to add overlapping maps, that is not something that can
    // happen in the real world.
    auto entry = map_ends.upper_bound(start);
    if (entry != map_ends.end() && end > entry->second) {
      continue;
    }
    map_ends[end] = start;

    uint64_t offset;
    if (!AlignToPage(data_provider->ConsumeIntegral<uint64_t>(), &offset)) {
      // Overflowed.
      continue;
    }
    std::string map_info_name = data_provider->ConsumeRandomLengthString(kMaxMapInfoNameLen);
    uint8_t flags = PROT_READ | PROT_WRITE;

    bool exec = data_provider->ConsumeBool();
    if (exec) {
      flags |= PROT_EXEC;
    }

    bool shouldAddElf = data_provider->ConsumeBool();
    if (shouldAddElf) {
      ElfAddMapInfo(maps.get(), start, end, offset, flags, map_info_name.c_str(),
                    PopulateElfFake(data_provider));
    } else {
      ElfAddMapInfo(maps.get(), start, end, offset, flags, map_info_name.c_str());
    }
  }
  maps->Sort();
  return maps;
}

// This code (until PutElfFilesInMemory) is pretty much directly copied from JitDebugTest.cpp
// There's a few minor modifications, most notably, all methods accept a MemoryFake pointer, and
// PutElfInMemory inserts JIT data when called.
void WriteDescriptor32(MemoryFake* memory, uint64_t addr, uint32_t entry) {
  // Format of the 32 bit JITDescriptor structure:
  //   uint32_t version
  memory->SetData32(addr, 1);
  //   uint32_t action_flag
  memory->SetData32(addr + 4, 0);
  //   uint32_t relevant_entry
  memory->SetData32(addr + 8, 0);
  //   uint32_t first_entry
  memory->SetData32(addr + 12, entry);
}

void WriteDescriptor64(MemoryFake* memory, uint64_t addr, uint64_t entry) {
  // Format of the 64 bit JITDescriptor structure:
  //   uint32_t version
  memory->SetData32(addr, 1);
  //   uint32_t action_flag
  memory->SetData32(addr + 4, 0);
  //   uint64_t relevant_entry
  memory->SetData64(addr + 8, 0);
  //   uint64_t first_entry
  memory->SetData64(addr + 16, entry);
}

void WriteEntry32Pack(MemoryFake* memory, uint64_t addr, uint32_t prev, uint32_t next,
                      uint32_t elf_addr, uint64_t elf_size) {
  // Format of the 32 bit JITCodeEntry structure:
  //   uint32_t next
  memory->SetData32(addr, next);
  //   uint32_t prev
  memory->SetData32(addr + 4, prev);
  //   uint32_t symfile_addr
  memory->SetData32(addr + 8, elf_addr);
  //   uint64_t symfile_size
  memory->SetData64(addr + 12, elf_size);
}

void WriteEntry32Pad(MemoryFake* memory, uint64_t addr, uint32_t prev, uint32_t next,
                     uint32_t elf_addr, uint64_t elf_size) {
  // Format of the 32 bit JITCodeEntry structure:
  //   uint32_t next
  memory->SetData32(addr, next);
  //   uint32_t prev
  memory->SetData32(addr + 4, prev);
  //   uint32_t symfile_addr
  memory->SetData32(addr + 8, elf_addr);
  //   uint32_t pad
  memory->SetData32(addr + 12, 0);
  //   uint64_t symfile_size
  memory->SetData64(addr + 16, elf_size);
}

void WriteEntry64(MemoryFake* memory, uint64_t addr, uint64_t prev, uint64_t next,
                  uint64_t elf_addr, uint64_t elf_size) {
  // Format of the 64 bit JITCodeEntry structure:
  //   uint64_t next
  memory->SetData64(addr, next);
  //   uint64_t prev
  memory->SetData64(addr + 8, prev);
  //   uint64_t symfile_addr
  memory->SetData64(addr + 16, elf_addr);
  //   uint64_t symfile_size
  memory->SetData64(addr + 24, elf_size);
}

template <typename EhdrType, typename ShdrType>
void PutElfInMemory(MemoryFake* memory, uint64_t offset, uint8_t class_type, uint8_t machine_type,
                    uint32_t pc, uint32_t size) {
  EhdrType ehdr;
  memset(&ehdr, 0, sizeof(ehdr));
  uint64_t sh_offset = sizeof(ehdr);
  memcpy(ehdr.e_ident, ELFMAG, SELFMAG);
  ehdr.e_ident[EI_CLASS] = class_type;
  ehdr.e_machine = machine_type;
  ehdr.e_shstrndx = 1;
  ehdr.e_shoff = sh_offset;
  ehdr.e_shentsize = sizeof(ShdrType);
  ehdr.e_shnum = 3;
  memory->SetMemory(offset, &ehdr, sizeof(ehdr));

  ShdrType shdr;
  memset(&shdr, 0, sizeof(shdr));
  shdr.sh_type = SHT_NULL;
  memory->SetMemory(offset + sh_offset, &shdr, sizeof(shdr));

  sh_offset += sizeof(shdr);
  memset(&shdr, 0, sizeof(shdr));
  shdr.sh_type = SHT_STRTAB;
  shdr.sh_name = 1;
  shdr.sh_offset = 0x500;
  shdr.sh_size = 0x100;
  memory->SetMemory(offset + sh_offset, &shdr, sizeof(shdr));
  memory->SetMemory(offset + 0x500, ".debug_frame");

  sh_offset += sizeof(shdr);
  memset(&shdr, 0, sizeof(shdr));
  shdr.sh_type = SHT_PROGBITS;
  shdr.sh_name = 0;
  shdr.sh_addr = 0x600;
  shdr.sh_offset = 0x600;
  shdr.sh_size = 0x200;
  memory->SetMemory(offset + sh_offset, &shdr, sizeof(shdr));

  // Now add a single cie/fde.
  uint64_t dwarf_offset = offset + 0x600;
  if (class_type == ELFCLASS32) {
    // CIE 32 information.
    memory->SetData32(dwarf_offset, 0xfc);
    memory->SetData32(dwarf_offset + 0x4, 0xffffffff);
    memory->SetData8(dwarf_offset + 0x8, 1);
    memory->SetData8(dwarf_offset + 0x9, '\0');
    memory->SetData8(dwarf_offset + 0xa, 0x4);
    memory->SetData8(dwarf_offset + 0xb, 0x4);
    memory->SetData8(dwarf_offset + 0xc, 0x1);

    // FDE 32 information.
    memory->SetData32(dwarf_offset + 0x100, 0xfc);
    memory->SetData32(dwarf_offset + 0x104, 0);
    memory->SetData32(dwarf_offset + 0x108, pc);
    memory->SetData32(dwarf_offset + 0x10c, size);
  } else {
    // CIE 64 information.
    memory->SetData32(dwarf_offset, 0xffffffff);
    memory->SetData64(dwarf_offset + 4, 0xf4);
    memory->SetData64(dwarf_offset + 0xc, 0xffffffffffffffffULL);
    memory->SetData8(dwarf_offset + 0x14, 1);
    memory->SetData8(dwarf_offset + 0x15, '\0');
    memory->SetData8(dwarf_offset + 0x16, 0x4);
    memory->SetData8(dwarf_offset + 0x17, 0x4);
    memory->SetData8(dwarf_offset + 0x18, 0x1);

    // FDE 64 information.
    memory->SetData32(dwarf_offset + 0x100, 0xffffffff);
    memory->SetData64(dwarf_offset + 0x104, 0xf4);
    memory->SetData64(dwarf_offset + 0x10c, 0);
    memory->SetData64(dwarf_offset + 0x114, pc);
    memory->SetData64(dwarf_offset + 0x11c, size);
  }
}

void PutElfFilesInMemory(MemoryFake* memory, FuzzedDataProvider* data_provider) {
  uint8_t elf_file_count = data_provider->ConsumeIntegralInRange<uint8_t>(0, kMaxJitElfFiles);
  int entry_offset = 0;
  int prev_jit_addr = 0;
  for (uint8_t i = 0; i < elf_file_count; i++) {
    uint64_t offset = data_provider->ConsumeIntegral<uint64_t>();
    // Technically the max valid value is ELFCLASSNUM - 1 (2), but
    // we want to test values outside of that range.
    uint8_t class_type = data_provider->ConsumeIntegral<uint8_t>();
    // Same here, EM_NUM is 253, max valid machine type is 252
    uint8_t machine_type = data_provider->ConsumeIntegral<uint8_t>();
    uint32_t pc = data_provider->ConsumeIntegral<uint32_t>();
    uint32_t size = data_provider->ConsumeIntegral<uint32_t>();
    bool sixty_four_bit = data_provider->ConsumeBool();
    bool write_jit = data_provider->ConsumeBool();
    if (sixty_four_bit) {
      PutElfInMemory<Elf64_Ehdr, Elf64_Shdr>(memory, offset, class_type, machine_type, pc, size);
    } else {
      PutElfInMemory<Elf32_Ehdr, Elf32_Shdr>(memory, offset, class_type, machine_type, pc, size);
    }
    if (write_jit) {
      bool use_pad = data_provider->ConsumeBool();
      // It is possible this will overwrite part of the ELF.
      // This provides an interesting test of how malformed ELF
      // data is handled.
      uint64_t cur_descriptor_addr = 0x11800 + entry_offset;
      uint64_t cur_jit_addr = 0x200000 + entry_offset;
      uint64_t next_jit_addr = cur_jit_addr + size;
      if (sixty_four_bit) {
        WriteDescriptor64(memory, 0x11800, cur_jit_addr);
        WriteEntry64(memory, cur_jit_addr, prev_jit_addr, next_jit_addr, pc, size);
      } else {
        // Loop back. Again, this may corrupt data,
        // but that will allow for testing edge cases with
        // malformed JIT data.
        if (cur_jit_addr > UINT32_MAX) {
          entry_offset = 0;
          cur_jit_addr = 0x200000;
          cur_descriptor_addr = 0x11800;
          next_jit_addr = cur_jit_addr + size;
        }
        WriteDescriptor32(memory, cur_descriptor_addr, cur_jit_addr);
        if (use_pad) {
          WriteEntry32Pad(memory, cur_jit_addr, prev_jit_addr, next_jit_addr, pc, size);
        } else {
          WriteEntry32Pack(memory, cur_jit_addr, prev_jit_addr, next_jit_addr, pc, size);
        }
      }
      entry_offset += size;
      prev_jit_addr = cur_jit_addr;
    }
  }
}

std::vector<std::string> GetStringList(FuzzedDataProvider* data_provider, uint max_str_len,
                                       uint max_strings) {
  uint str_count = data_provider->ConsumeIntegralInRange<uint>(0, max_strings);
  std::vector<std::string> strings;
  for (uint i = 0; i < str_count; i++) {
    strings.push_back(data_provider->ConsumeRandomLengthString(max_str_len));
  }
  return strings;
}

std::unique_ptr<DexFiles> GetDexFiles(FuzzedDataProvider* data_provider,
                                      std::shared_ptr<Memory> memory, uint max_library_length,
                                      uint max_libraries, ArchEnum arch) {
  std::vector<std::string> search_libs =
      GetStringList(data_provider, max_library_length, max_libraries);
  if (search_libs.size() <= 0) {
    return CreateDexFiles(arch, memory);
  }

  return CreateDexFiles(arch, memory, search_libs);
}
