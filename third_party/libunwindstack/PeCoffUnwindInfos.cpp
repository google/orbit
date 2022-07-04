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

#include "PeCoffUnwindInfos.h"

#include <memory>
#include <unordered_map>
#include <utility>
#include "Check.h"

namespace unwindstack {

class PeCoffUnwindInfosImpl : public PeCoffUnwindInfos {
 public:
  explicit PeCoffUnwindInfosImpl(Memory* memory, std::vector<Section> sections)
      : pe_coff_memory_(memory), sections_(std::move(sections)) {}

  bool GetUnwindInfo(uint64_t unwind_info_rva, UnwindInfo** unwind_info) override;

 private:
  bool MapFromRVAToFileOffset(uint64_t rva, uint64_t* file_offset);
  bool ParseUnwindInfoAtOffset(uint64_t file_offset, UnwindInfo* unwind_info);

  PeCoffMemory pe_coff_memory_;
  // This is a cache of unwind infos.
  std::unordered_map<uint64_t, UnwindInfo> unwind_info_rva_to_unwind_info_;

  std::vector<Section> sections_;
};

std::unique_ptr<PeCoffUnwindInfos> CreatePeCoffUnwindInfos(Memory* memory,
                                                           std::vector<Section> sections) {
  return std::make_unique<PeCoffUnwindInfosImpl>(memory, std::move(sections));
}

bool PeCoffUnwindInfosImpl::MapFromRVAToFileOffset(uint64_t rva, uint64_t* file_offset) {
  for (auto& section : sections_) {
    if (section.vmaddr <= rva && rva < section.vmaddr + section.vmsize) {
      *file_offset = rva - section.vmaddr + section.offset;
      return true;
    }
  }
  last_error_.code = ERROR_INVALID_COFF;
  return false;
}

bool PeCoffUnwindInfosImpl::GetUnwindInfo(uint64_t unwind_info_rva, UnwindInfo** unwind_info) {
  const auto& it = unwind_info_rva_to_unwind_info_.find(unwind_info_rva);
  if (it != unwind_info_rva_to_unwind_info_.end()) {
    *unwind_info = &it->second;
    return true;
  }

  uint64_t file_offset;
  if (!MapFromRVAToFileOffset(unwind_info_rva, &file_offset)) {
    last_error_.code = ERROR_INVALID_COFF;
    return false;
  }

  UnwindInfo new_unwind_info;
  if (!ParseUnwindInfoAtOffset(file_offset, &new_unwind_info)) {
    return false;
  }

  const auto& [new_it, inserted] =
      unwind_info_rva_to_unwind_info_.emplace(unwind_info_rva, std::move(new_unwind_info));
  CHECK(inserted);
  *unwind_info = &new_it->second;
  return true;
}

bool PeCoffUnwindInfosImpl::ParseUnwindInfoAtOffset(uint64_t offset, UnwindInfo* unwind_info) {
  // Need to remember the original offset for later
  const uint64_t base_offset = offset;

  constexpr uint64_t kUnwindInfoHeaderSize = 4;
  std::vector<uint8_t> data_info(kUnwindInfoHeaderSize);
  pe_coff_memory_.set_cur_offset(offset);
  if (!pe_coff_memory_.GetFully(static_cast<void*>(&data_info[0]), kUnwindInfoHeaderSize)) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = offset;
    return false;
  }

  unwind_info->version_and_flags = data_info[0];
  unwind_info->prolog_size = data_info[1];
  unwind_info->num_codes = data_info[2];
  unwind_info->frame_register_and_offset = data_info[3];

  // Version 1 is the only documented one:
  // https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64#struct-unwind_info
  // However, there is also an undocumented version 2 that adds the UWOP_EPILOG opcode.
  if (unwind_info->GetVersion() != 0x01 && unwind_info->GetVersion() != 0x02) {
    last_error_.code = ERROR_INVALID_COFF;
    return false;
  }

  unwind_info->unwind_codes.resize(unwind_info->num_codes);
  pe_coff_memory_.set_cur_offset(offset + kUnwindInfoHeaderSize);
  if (!pe_coff_memory_.GetFully(static_cast<void*>(&unwind_info->unwind_codes[0]),
                                unwind_info->num_codes * sizeof(UnwindCode))) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = pe_coff_memory_.cur_offset();
    return false;
  }

  if (unwind_info->HasChainedInfo()) {
    // For alignment purposes, the unwind codes array always has an even number of entries,
    // with the last one potentially being unused (as indicated by num_codes). To find the
    // chained function (which is a RUNTIME_FUNCTION struct), we therefore need to round
    // the num_codes value to an even number. See also
    // https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64?view=msvc-170#chained-unwind-info-structures
    // for the source of the expression used below.
    const uint64_t runtime_function_offset =
        base_offset + kUnwindInfoHeaderSize +
        ((unwind_info->num_codes + 1) & ~1) * sizeof(UnwindCode);

    pe_coff_memory_.set_cur_offset(runtime_function_offset);
    if (!pe_coff_memory_.Get32(&(unwind_info->chained_info.start_address)) ||
        !pe_coff_memory_.Get32(&(unwind_info->chained_info.end_address)) ||
        !pe_coff_memory_.Get32(&(unwind_info->chained_info.unwind_info_offset))) {
      last_error_.code = ERROR_MEMORY_INVALID;
      last_error_.address = pe_coff_memory_.cur_offset();
      return false;
    }
  }
  return true;
}

}  // namespace unwindstack