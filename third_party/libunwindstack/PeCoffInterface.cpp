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

#include <unwindstack/PeCoffInterface.h>

#include <android-base/parseint.h>

#include <unwindstack/Error.h>
#include <unwindstack/Log.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Regs.h>
#include "Check.h"
#include "DwarfDebugFrame.h"
#include "PeCoffUnwindInfoUnwinderX86_64.h"

namespace unwindstack {

bool PeCoffMemory::Get8(uint8_t* value) {
  bool success = memory_->Read8(cur_offset_, value);
  if (success) {
    cur_offset_ += sizeof(uint8_t);
  }
  return success;
}

bool PeCoffMemory::Get16(uint16_t* value) {
  bool success = memory_->Read16(cur_offset_, value);
  if (success) {
    cur_offset_ += sizeof(uint16_t);
  }
  return success;
}

bool PeCoffMemory::Get32(uint32_t* value) {
  bool success = memory_->Read32(cur_offset_, value);
  if (success) {
    cur_offset_ += sizeof(uint32_t);
  }
  return success;
}

bool PeCoffMemory::Get64(uint64_t* value) {
  bool success = memory_->Read64(cur_offset_, value);
  if (success) {
    cur_offset_ += sizeof(uint64_t);
  }
  return success;
}

bool PeCoffMemory::GetMax64(uint64_t* value, uint64_t size) {
  switch (size) {
    case 1:
      uint8_t value8;
      if (!Get8(&value8)) {
        return false;
      }
      *value = value8;
      return true;
    case 2:
      uint16_t value16;
      if (!Get16(&value16)) {
        return false;
      }
      *value = value16;
      return true;
    case 4:
      uint32_t value32;
      if (!Get32(&value32)) {
        return false;
      }
      *value = value32;
      return true;
    case 8:
      return Get64(value);
    default:
      return false;
  }
  return false;
}

bool PeCoffMemory::GetFully(void* dst, size_t size) {
  bool success = memory_->ReadFully(cur_offset_, dst, size);
  if (success) {
    cur_offset_ += size;
  }
  return success;
}

template <typename AddressType>
bool PeCoffInterfaceImpl<AddressType>::ParseDosHeader(uint64_t offset) {
  coff_memory_.set_cur_offset(offset);
  if (!coff_memory_.Get16(&dos_header_.e_magic)) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = coff_memory_.cur_offset();
    return false;
  }

  constexpr uint16_t kMsDosTwoPointZeroMagicValue = 0x5a4d;
  if (dos_header_.e_magic != kMsDosTwoPointZeroMagicValue) {
    Log::Error("Magic MS-DOS 2.0 value not found. Value read: %x", dos_header_.e_magic);
    last_error_.code = ERROR_INVALID_COFF;
    return false;
  }

  // There is data in the DOS header that we don't need, but we still want to make sure that
  // we can correctly read the memory at these addresses.
  constexpr size_t kDosHeaderSize = 0x40;
  std::vector<uint8_t> unused_data(kDosHeaderSize - sizeof(uint16_t) - sizeof(uint32_t));
  if (!coff_memory_.GetFully(&unused_data[0], unused_data.size())) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = coff_memory_.cur_offset();
    return false;
  }

  if (!coff_memory_.Get32(&dos_header_.e_lfanew)) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = coff_memory_.cur_offset();
    return false;
  }
  return true;
}

template <typename AddressType>
bool PeCoffInterfaceImpl<AddressType>::ParseNewHeader(uint64_t offset) {
  coff_memory_.set_cur_offset(offset);
  uint32_t pe_signature;
  if (!coff_memory_.Get32(&pe_signature)) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = coff_memory_.cur_offset();
    return false;
  }
  constexpr uint32_t kImagePeSignature = 0x00004550;
  if (pe_signature != kImagePeSignature) {
    Log::Error("PE image signature not found");
    last_error_.code = ERROR_INVALID_COFF;
    return false;
  }
  return true;
}

template <typename AddressType>
bool PeCoffInterfaceImpl<AddressType>::ParseCoffHeader(uint64_t offset) {
  coff_memory_.set_cur_offset(offset);
  if (!coff_memory_.Get16(&coff_header_.machine) || !coff_memory_.Get16(&coff_header_.nsects) ||
      !coff_memory_.Get32(&coff_header_.modtime) || !coff_memory_.Get32(&coff_header_.symoff) ||
      !coff_memory_.Get32(&coff_header_.nsyms) || !coff_memory_.Get16(&coff_header_.hdrsize) ||
      !coff_memory_.Get16(&coff_header_.flags)) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = coff_memory_.cur_offset();
    Log::Error("Parsing the COFF header failed: %s", GetErrorCodeString(last_error_.code));
    return false;
  }
  return true;
}

template <typename AddressType>
bool PeCoffInterfaceImpl<AddressType>::ParseOptionalHeader(uint64_t offset) {
  const uint64_t optional_header_start_offset = offset;

  coff_memory_.set_cur_offset(offset);
  if (!coff_memory_.Get16(&optional_header_.magic) ||
      !coff_memory_.Get8(&optional_header_.major_linker_version) ||
      !coff_memory_.Get8(&optional_header_.minor_linker_version) ||
      !coff_memory_.Get32(&optional_header_.code_size) ||
      !coff_memory_.Get32(&optional_header_.data_size) ||
      !coff_memory_.Get32(&optional_header_.bss_size) ||
      !coff_memory_.Get32(&optional_header_.entry) ||
      !coff_memory_.Get32(&optional_header_.code_offset)) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = coff_memory_.cur_offset();
    Log::Error("Parsing the optional header failed: %s", GetErrorCodeString(last_error_.code));
    return false;
  }

  constexpr uint32_t kOptionalHeaderMagicPE32 = 0x010b;
  constexpr uint32_t kOptionalHeaderMagicPE32Plus = 0x020b;
  if (optional_header_.magic == kOptionalHeaderMagicPE32) {
    if (sizeof(AddressType) != 4) {
      Log::Error("Tried to initialize 64-bit PE/COFF interface with 32-bit PE/COFF file");
      last_error_.code = ERROR_UNSUPPORTED;
      return false;
    }
    if (!coff_memory_.Get32(&optional_header_.data_offset)) {
      Log::Error("Can't read data offset for 32-bit PE/COFF file");
      last_error_.code = ERROR_MEMORY_INVALID;
      last_error_.address = coff_memory_.cur_offset();
      return false;
    }
  } else if (optional_header_.magic == kOptionalHeaderMagicPE32Plus) {
    if (sizeof(AddressType) != 8) {
      Log::Error("Tried to initialize 32-bit PE/COFF interface with 64-bit PE/COFF file");
      last_error_.code = ERROR_UNSUPPORTED;
      return false;
    }
    optional_header_.data_offset = 0;
  } else {
    Log::Error("Magic PE value not found. Value read: %x", optional_header_.magic);
    last_error_.code = ERROR_INVALID_COFF;
    return false;
  }

  if (!coff_memory_.GetMax64(&optional_header_.image_base, sizeof(AddressType)) ||
      !coff_memory_.Get32(&optional_header_.sect_alignment) ||
      !coff_memory_.Get32(&optional_header_.file_alignment) ||
      !coff_memory_.Get16(&optional_header_.major_os_system_version) ||
      !coff_memory_.Get16(&optional_header_.minor_os_system_version) ||
      !coff_memory_.Get16(&optional_header_.major_image_version) ||
      !coff_memory_.Get16(&optional_header_.minor_image_version) ||
      !coff_memory_.Get16(&optional_header_.major_subsystem_version) ||
      !coff_memory_.Get16(&optional_header_.minor_subsystem_version) ||
      !coff_memory_.Get32(&optional_header_.reserved1) ||
      !coff_memory_.Get32(&optional_header_.image_size) ||
      !coff_memory_.Get32(&optional_header_.header_size) ||
      !coff_memory_.Get32(&optional_header_.checksum) ||
      !coff_memory_.Get16(&optional_header_.subsystem) ||
      !coff_memory_.Get16(&optional_header_.dll_flags) ||
      !coff_memory_.GetMax64(&optional_header_.stack_reserve_size, sizeof(AddressType)) ||
      !coff_memory_.GetMax64(&optional_header_.stack_commit_size, sizeof(AddressType)) ||
      !coff_memory_.GetMax64(&optional_header_.heap_reserve_size, sizeof(AddressType)) ||
      !coff_memory_.GetMax64(&optional_header_.heap_commit_size, sizeof(AddressType)) ||
      !coff_memory_.Get32(&optional_header_.loader_flags) ||
      !coff_memory_.Get32(&optional_header_.num_data_dir_entries)) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = coff_memory_.cur_offset();
    Log::Error("Parsing the optional header failed: %s", GetErrorCodeString(last_error_.code));
    return false;
  }

  // We check if hdrsize (which is the size of the optional header) and num_data_dir_entries are
  // consistent with each other. The remaining size of data according to hdrsize must match exactly
  // the size of the data directory entries. If not, the COFF file is invalid.
  constexpr uint64_t size_per_data_dir_entry = 2 * sizeof(uint32_t);
  const uint64_t end_offset = optional_header_start_offset + coff_header_.hdrsize;
  const uint64_t expected_num_data_dir_entries_size = (end_offset - coff_memory_.cur_offset());
  if (expected_num_data_dir_entries_size !=
      size_per_data_dir_entry * optional_header_.num_data_dir_entries) {
    last_error_.code = ERROR_INVALID_COFF;
    Log::Error("Optional header size or number of data directories is incorrect");
    return false;
  }

  optional_header_.data_dirs.clear();
  optional_header_.data_dirs.resize(optional_header_.num_data_dir_entries);
  for (uint32_t i = 0; i < optional_header_.num_data_dir_entries; ++i) {
    if (!coff_memory_.Get32(&optional_header_.data_dirs[i].vm_addr) ||
        !coff_memory_.Get32(&optional_header_.data_dirs[i].vm_size)) {
      last_error_.code = ERROR_MEMORY_INVALID;
      last_error_.address = coff_memory_.cur_offset();
      Log::Error("Parsing error when reading data directories: %s",
                 GetErrorCodeString(last_error_.code));
      return false;
    }
  }

  coff_memory_.set_cur_offset(end_offset);

  return true;
}

template <typename AddressType>
bool PeCoffInterfaceImpl<AddressType>::ParseSectionHeaders(uint64_t offset) {
  const uint16_t num_sections = coff_header_.nsects;

  coff_memory_.set_cur_offset(offset);

  for (uint16_t idx = 0; idx < num_sections; ++idx) {
    SectionHeader section_header;

    // Section names in the header are always exactly kSectionNameInHeaderSize (== 8) bytes. Longer
    // names have to be looked up in the string table.
    if (!coff_memory_.GetFully(static_cast<void*>(&section_header.name[0]),
                               kSectionNameInHeaderSize)) {
      last_error_.code = ERROR_MEMORY_INVALID;
      last_error_.address = coff_memory_.cur_offset();
      Log::Error("Parsing section header failed: %s", GetErrorCodeString(last_error_.code));
      return false;
    }

    if (!coff_memory_.Get32(&section_header.vmsize) ||
        !coff_memory_.Get32(&section_header.vmaddr) || !coff_memory_.Get32(&section_header.size) ||
        !coff_memory_.Get32(&section_header.offset) ||
        !coff_memory_.Get32(&section_header.reloff) ||
        !coff_memory_.Get32(&section_header.lineoff) || !coff_memory_.Get16(&section_header.nrel) ||
        !coff_memory_.Get16(&section_header.nline) || !coff_memory_.Get32(&section_header.flags)) {
      last_error_.code = ERROR_MEMORY_INVALID;
      last_error_.address = coff_memory_.cur_offset();
      Log::Error("Parsing section header failed: %s", GetErrorCodeString(last_error_.code));
      return false;
    }
    parsed_section_headers_.emplace_back(section_header);
  }
  return true;
}

template <typename AddressType>
bool PeCoffInterfaceImpl<AddressType>::GetSectionName(const std::string& parsed_section_name_string,
                                                      std::string* result) {
  uint64_t offset = 0;
  if (!android::base::ParseUint<uint64_t>(parsed_section_name_string.c_str(), &offset)) {
    Log::Error("Failed to parse section name as integer: %s", parsed_section_name_string.c_str());
    last_error_.code = ERROR_INVALID_COFF;
    return false;
  }

  // The symbols come first and every one of them has a size of 18 bytes and we need to add
  // this to the offset to get to the strings for the section names.
  const uint64_t file_offset = coff_header_.symoff + (18 * coff_header_.nsyms) + offset;

  // Arbitrarily chosen to be large enough.
  constexpr uint64_t kMaxSectionNameLength = 1024;

  if (!coff_memory_.ReadString(file_offset, result, kMaxSectionNameLength)) {
    Log::Error("GetSectionName() failed when reading section name string");
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = file_offset;
    return false;
  }
  return true;
}

template <typename AddressType>
bool PeCoffInterfaceImpl<AddressType>::InitSections() {
  for (const auto& section_header : parsed_section_headers_) {
    Section section;
    const std::string header_name(section_header.name.data(), section_header.name.size());
    std::string name_trimmed = header_name.substr(0, header_name.find('\0'));
    if (name_trimmed[0] != '/') {
      section.name = name_trimmed;
    } else if (!GetSectionName(name_trimmed.substr(1), &section.name)) {
      return false;
    }
    section.vmaddr = section_header.vmaddr;
    section.vmsize = section_header.vmsize;
    section.offset = section_header.offset;
    section.size = section_header.size;
    section.flags = section_header.flags;
    sections_.emplace_back(section);
  }

  for (size_t i = 0; i < sections_.size(); ++i) {
    // Find the .text section as the first section with characteristics IMAGE_SCN_CNT_CODE and
    // IMAGE_SCN_MEM_EXECUTE. We prefer this to looking for a section with name ".text", because we
    // have observed that this is not very reliable: for example, changing the section names can be
    // used as a simple means of obfuscation.
    constexpr uint32_t kImageScnCntCode = 0x00000020;
    constexpr uint32_t kImageScnMemExecute = 0x20000000;
    if (!text_section_data_.has_value() && (sections_[i].flags & kImageScnCntCode) != 0 &&
        (sections_[i].flags & kImageScnMemExecute) != 0) {
      TextSectionData section_data;
      section_data.memory_size = sections_[i].vmsize;
      section_data.memory_offset = sections_[i].vmaddr;
      section_data.file_offset = sections_[i].offset;
      text_section_data_.emplace(section_data);
    }

    if (sections_[i].name == ".debug_frame") {
      DebugFrameSectionData section_data;
      section_data.file_offset = sections_[i].offset;
      section_data.size = sections_[i].vmsize;
      uint64_t debug_frame_section_bias =
          static_cast<int64_t>(sections_[i].vmaddr) - sections_[i].offset;
      section_data.section_bias = debug_frame_section_bias;
      debug_frame_section_data_.emplace(section_data);
    }
  }

  if (!text_section_data_.has_value()) {
    Log::Error("PE/COFF object file does not have a .text section");
    last_error_.code = ERROR_INVALID_COFF;
    return false;
  }

  return true;
}

template <typename AddressType>
bool PeCoffInterfaceImpl<AddressType>::InitDebugFrameSection() {
  CHECK(debug_frame_section_data_.has_value());

  debug_frame_.reset(new DwarfDebugFrame<AddressType>(memory_));

  if (!debug_frame_->Init(debug_frame_section_data_->file_offset, debug_frame_section_data_->size,
                          debug_frame_section_data_->section_bias)) {
    debug_frame_.reset(nullptr);
    debug_frame_section_data_->file_offset = 0;
    debug_frame_section_data_->size = 0;
    debug_frame_section_data_->section_bias = 0;

    Log::Error("Failed to initialize the .debug_frame section for PE/COFF file.");
    last_error_.code = ERROR_INVALID_COFF;
    return false;
  }
  return true;
}

template <typename AddressType>
bool PeCoffInterfaceImpl<AddressType>::ParseAllHeaders() {
  if (!ParseDosHeader(0x0)) {
    return false;
  }
  if (!ParseNewHeader(dos_header_.e_lfanew)) {
    return false;
  }
  if (!ParseCoffHeader(coff_memory_.cur_offset())) {
    return false;
  }

  if (coff_header_.hdrsize > 0 && !ParseOptionalHeader(coff_memory_.cur_offset())) {
    return false;
  }

  if (!ParseSectionHeaders(coff_memory_.cur_offset())) {
    return false;
  }
  return true;
}

template <typename AddressType>
bool PeCoffInterfaceImpl<AddressType>::Init(int64_t* load_bias) {
  if (!ParseAllHeaders()) {
    return false;
  }
  if (!InitSections()) {
    return false;
  }
  if (debug_frame_section_data_.has_value() && !InitDebugFrameSection()) {
    // If initializing the debug frame section fails, we assume that the PE/COFF file
    // is corrupted, consider it invalid and therefore abort initialization.
    return false;
  }

  // Only initialize the native unwinder in the 64-bit case.
  constexpr uint32_t kOptionalHeaderMagicPE32Plus = 0x020b;
  if ((optional_header_.magic == kOptionalHeaderMagicPE32Plus) && !InitNativeUnwinder()) {
    return false;
  }

  if (optional_header_.image_base > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
    Log::Error("Value of ImageBase in PE/COFF file is too large.");
    return false;
  }
  *load_bias = static_cast<int64_t>(optional_header_.image_base);
  return true;
}

template <typename AddressType>
bool PeCoffInterfaceImpl<AddressType>::MapFromRvaToFileOffset(uint64_t rva, uint64_t* file_offset) {
  for (const Section& section : sections_) {
    if (section.vmaddr <= rva && rva < section.vmaddr + section.vmsize) {
      *file_offset = rva - section.vmaddr + section.offset;
      return true;
    }
  }
  last_error_.code = ERROR_INVALID_COFF;
  return false;
}

template <typename AddressType>
bool PeCoffInterfaceImpl<AddressType>::MapFromFileOffsetToRva(uint64_t file_offset, uint64_t* rva) {
  for (const Section& section : sections_) {
    if (section.offset <= file_offset && file_offset < section.offset + section.size) {
      *rva = file_offset - section.offset + section.vmaddr;
      return true;
    }
  }
  last_error_.code = ERROR_INVALID_COFF;
  return false;
}

template <>
bool PeCoffInterfaceImpl<uint64_t>::InitNativeUnwinder() {
  constexpr int kCoffDataDirExceptionTableIndex = 3;
  if (kCoffDataDirExceptionTableIndex >= optional_header_.data_dirs.size()) {
    last_error_.code = ERROR_INVALID_COFF;
    return false;
  }
  DataDirectory data_directory = optional_header_.data_dirs[kCoffDataDirExceptionTableIndex];
  if (data_directory.vm_addr == 0) {
    return false;
  }
  constexpr uint16_t kImageFileMachineAmd64 = 0x8664;
  if (coff_header_.machine != kImageFileMachineAmd64) {
    last_error_.code = ERROR_INVALID_COFF;
    return false;
  }

  uint32_t rva = data_directory.vm_addr;
  uint32_t size = data_directory.vm_size;

  uint64_t pdata_file_begin;
  if (!MapFromRvaToFileOffset(rva, &pdata_file_begin)) {
    return false;
  }
  uint64_t pdata_file_end = pdata_file_begin + size;

  if (!text_section_data_.has_value()) {
    return false;
  }

  native_unwinder_ = std::make_unique<PeCoffUnwindInfoUnwinderX86_64>(
      memory_, optional_header_.image_base, pdata_file_begin, pdata_file_end, sections_);
  return native_unwinder_->Init();
}

template <>
bool PeCoffInterfaceImpl<uint32_t>::InitNativeUnwinder() {
  return false;
}

template <typename AddressType>
uint64_t PeCoffInterfaceImpl<AddressType>::GetRelPcWithMapOffset(uint64_t pc, uint64_t map_start,
                                                                 uint64_t map_object_offset) {
  uint64_t map_rva;
  if (!MapFromFileOffsetToRva(map_object_offset, &map_rva)) {
    return 0;
  }
  return pc - map_start + optional_header_.image_base + map_rva;
}

template <typename AddressType>
uint64_t PeCoffInterfaceImpl<AddressType>::GetRelPcWithMapRva(uint64_t pc, uint64_t map_start,
                                                              uint64_t map_object_rva) {
  return pc - map_start + optional_header_.image_base + map_object_rva;
}

template <typename AddressType>
bool PeCoffInterfaceImpl<AddressType>::GetTextRange(uint64_t* addr, uint64_t* size) const {
  if (!text_section_data_.has_value()) {
    return false;
  }

  *addr = text_section_data_->memory_offset;
  *size = text_section_data_->memory_size;
  return true;
}

template <typename AddressType>
uint64_t PeCoffInterfaceImpl<AddressType>::GetTextOffsetInFile() const {
  if (!text_section_data_.has_value()) {
    return 0;
  }
  return text_section_data_->file_offset;
}

template <typename AddressType>
uint64_t PeCoffInterfaceImpl<AddressType>::GetSizeOfImage() const {
  return optional_header_.image_size;
}

template <typename AddressType>
bool PeCoffInterfaceImpl<AddressType>::Step(uint64_t rel_pc, uint64_t pc_adjustment, Regs* regs,
                                            Memory* process_memory, bool* finished,
                                            bool* is_signal_frame) {
  *is_signal_frame = false;
  // Try the debug_frame first since it contains the most specific and comprehensive unwind
  // information.
  if (debug_frame_ && debug_frame_->Step(rel_pc, regs, process_memory, finished, is_signal_frame)) {
    return true;
  }

  if (native_unwinder_ && native_unwinder_->Step(rel_pc, pc_adjustment, regs, process_memory,
                                                 finished, is_signal_frame)) {
    return true;
  }
  return false;
}

// Instantiate all of the needed template functions.
template class PeCoffInterfaceImpl<uint32_t>;
template class PeCoffInterfaceImpl<uint64_t>;
}  // namespace unwindstack
