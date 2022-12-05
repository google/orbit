// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ExtractSignalFromMinidump.h"

#include <cstdint>
#include <string>
#include <string_view>

#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"

namespace orbit_linux_capture_service {

namespace {

// The constants and structs below help to interpret the minidump file format. The data here is
// limited to access the part of the file that we are interested in. For a complete parser see
// minidump-2-core.cc from https://chromium.googlesource.com/breakpad/breakpad/.

uint32_t kStreamCountOffset = 8;
uint32_t kStreamDirectoryOffset = 12;
uint32_t kMdExceptionStream = 6;

struct MDLocationDescriptor {
  uint32_t data_size;
  uint32_t rva;
};

struct MDRawDirectory {
  uint32_t stream_type;
  MDLocationDescriptor location;
};

struct MDException {
  uint32_t exception_code;
  uint32_t exception_flags;
  uint64_t exception_record;
  uint64_t exception_address;
  uint32_t number_parameters;
  uint32_t __align;
  uint64_t exception_information[15];
};

struct MDRawExceptionStream {
  uint32_t thread_id;
  uint32_t __align;
  MDException exception_record;
  MDLocationDescriptor thread_context;
};

template <typename T>
ErrorMessageOr<const T*> DataOrError(std::string_view content, uint64_t offset) {
  if (content.size() < offset + sizeof(T)) {
    return ErrorMessage("Unexpected end of data.");
  }
  return reinterpret_cast<const T*>(content.data() + offset);
}

template <typename T>
ErrorMessageOr<const T*> ArrayElementOrError(std::string_view content, uint64_t offset,
                                             uint64_t index) {
  return DataOrError<T>(content, offset + index * sizeof(T));
}

ErrorMessageOr<int32_t> ParseMinidumpForTerminationSignal(std::string_view content) {
  const uint32_t* stream_count = nullptr;
  OUTCOME_TRY(stream_count, DataOrError<uint32_t>(content, kStreamCountOffset));
  const uint32_t* stream_directory = nullptr;
  OUTCOME_TRY(stream_directory, DataOrError<uint32_t>(content, kStreamDirectoryOffset));
  for (uint32_t i = 0; i < *stream_count; i++) {
    const MDRawDirectory* dir_entry = nullptr;
    OUTCOME_TRY(dir_entry, ArrayElementOrError<MDRawDirectory>(content, *stream_directory, i));
    if (dir_entry->stream_type != kMdExceptionStream) {
      continue;
    }
    const MDRawExceptionStream* exception_stream = nullptr;
    OUTCOME_TRY(exception_stream,
                DataOrError<MDRawExceptionStream>(content, dir_entry->location.rva));
    return exception_stream->exception_record.exception_code;
  }
  return ErrorMessage("No termination signal found in core file.");
}

}  // namespace

ErrorMessageOr<int> ExtractSignalFromMinidump(const std::filesystem::path& path) {
  OUTCOME_TRY(std::string content, orbit_base::ReadFileToString(path));
  OUTCOME_TRY(int result, ParseMinidumpForTerminationSignal(content));
  constexpr int kMinValidSignal = 1;
  constexpr int kMaxValidSignal = 31;
  if (result < kMinValidSignal || result > kMaxValidSignal) {
    return ErrorMessage("Found invalid signal in core file.");
  }
  return result;
}

}  // namespace orbit_linux_capture_service