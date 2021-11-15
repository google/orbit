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

#include <unwindstack/Error.h>
#include <unwindstack/Log.h>
#include <unwindstack/Memory.h>

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

template <typename AddressType>
bool PeCoffInterface<AddressType>::ParseDosHeader(uint64_t offset) {
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
  if (!coff_memory_.ReadFully(coff_memory_.cur_offset(), &unused_data[0], unused_data.size())) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = coff_memory_.cur_offset();
    return false;
  }

  coff_memory_.set_cur_offset(offset + 0x3c);
  if (!coff_memory_.Get32(&dos_header_.e_lfanew)) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = coff_memory_.cur_offset();
    return false;
  }
  return true;
}

template <typename AddressType>
bool PeCoffInterface<AddressType>::ParseNewHeader(uint64_t offset) {
  coff_memory_.set_cur_offset(offset);
  uint32_t pe_signature;
  if (!coff_memory_.Get32(&pe_signature)) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = coff_memory_.cur_offset();
    return false;
  }
  uint32_t kImagePeSignature = 0x00004550;
  if (pe_signature != kImagePeSignature) {
    Log::Error("PE image signature not found");
    last_error_.code = ERROR_INVALID_COFF;
    return false;
  }
  return true;
}

template <typename AddressType>
bool PeCoffInterface<AddressType>::ParseCoffHeader(uint64_t offset) {
  coff_memory_.set_cur_offset(offset);
  if (!coff_memory_.Get16(&coff_header_.machine) || !coff_memory_.Get32(&coff_header_.modtime) ||
      !coff_memory_.Get32(&coff_header_.symoff) || !coff_memory_.Get32(&coff_header_.nsyms) ||
      !coff_memory_.Get16(&coff_header_.hdrsize) || !coff_memory_.Get16(&coff_header_.flags)) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = coff_memory_.cur_offset();
    Log::Error("Parsing the COFF header failed: %s", GetErrorCodeString(last_error_.code));
    return false;
  }
  return true;
}

template <typename AddressType>
bool PeCoffInterface<AddressType>::ParseOptionalHeader(uint64_t offset) {
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

  return true;
}

template <typename AddressType>
bool PeCoffInterface<AddressType>::ParseAllHeaders() {
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
  // TODO: Parse section headers.
  // if (!ParseSectionHeaders(coff_header_, memory, &offset, &section_headers_)) {
  //   return false;
  // }
  return true;
}

template <typename AddressType>
bool PeCoffInterface<AddressType>::Init() {
  return ParseAllHeaders();
}

// Instantiate all of the needed template functions.
template class PeCoffInterface<uint32_t>;
template class PeCoffInterface<uint64_t>;
}  // namespace unwindstack