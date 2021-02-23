// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_MODEL_CAPTURE_SERIALIZER_H_
#define ORBIT_CLIENT_MODEL_CAPTURE_SERIALIZER_H_

#include <absl/container/flat_hash_map.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>
#include <stdint.h>

#include <filesystem>
#include <outcome.hpp>
#include <string>

#include "CaptureData.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "capture_data.pb.h"

namespace capture_serializer {

template <class TimersIterator>
ErrorMessageOr<void> Save(const std::filesystem::path& filename, const CaptureData& capture_data,
                          const absl::flat_hash_map<uint64_t, std::string>& key_to_string_map,
                          TimersIterator timers_iterator_begin, TimersIterator timers_iterator_end);

void WriteMessage(const google::protobuf::Message* message,
                  google::protobuf::io::CodedOutputStream* output);

std::string GetCaptureFileName(const CaptureData& capture_data);

void IncludeOrbitExtensionInFile(std::string& file_name);

namespace internal {

inline const std::string kRequiredCaptureVersion = "1.59";

orbit_client_protos::CaptureInfo GenerateCaptureInfo(
    const CaptureData& capture_data,
    const absl::flat_hash_map<uint64_t, std::string>& key_to_string_map);

template <class TimersIterator>
void Save(google::protobuf::io::CodedOutputStream* coded_output, const CaptureData& capture_data,
          const absl::flat_hash_map<uint64_t, std::string>& key_to_string_map,
          TimersIterator timers_iterator_begin, TimersIterator timers_iterator_end) {
  orbit_client_protos::CaptureHeader header;
  header.set_version(kRequiredCaptureVersion);
  WriteMessage(&header, coded_output);

  orbit_client_protos::CaptureInfo capture_info =
      GenerateCaptureInfo(capture_data, key_to_string_map);
  WriteMessage(&capture_info, coded_output);

  // Timers
  for (auto it = timers_iterator_begin; it != timers_iterator_end; ++it) {
    WriteMessage(&(*it), coded_output);
  }
}

}  // namespace internal

template <class TimersIterator>
ErrorMessageOr<void> Save(const std::filesystem::path& filename, const CaptureData& capture_data,
                          const absl::flat_hash_map<uint64_t, std::string>& key_to_string_map,
                          TimersIterator timers_iterator_begin,
                          TimersIterator timers_iterator_end) {
  auto fd_or_error = orbit_base::OpenFileForWriting(filename);

  if (fd_or_error.has_error()) {
    ERROR("%s", fd_or_error.error().message());
    return fd_or_error.error();
  }

  google::protobuf::io::FileOutputStream out_stream(fd_or_error.value());
  google::protobuf::io::CodedOutputStream coded_output(&out_stream);

  {
    SCOPED_TIMED_LOG("Saving capture in \"%s\"", filename.string());
    internal::Save(&coded_output, capture_data, key_to_string_map, std::move(timers_iterator_begin),
                   std::move(timers_iterator_end));
  }

  return outcome::success();
}

}  // namespace capture_serializer

#endif  // ORBIT_CLIENT_MODEL_CAPTURE_SERIALIZER_H_
