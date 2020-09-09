// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_MODEL_CAPTURE_SERIALIZER_H_
#define ORBIT_CLIENT_MODEL_CAPTURE_SERIALIZER_H_

#include <iosfwd>
#include <outcome.hpp>
#include <string>

#include "CaptureData.h"
#include "OrbitBase/Result.h"
#include "capture_data.pb.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message.h"

namespace capture_serializer {

template <class TimersIterator>
ErrorMessageOr<void> Save(const std::string& filename, const CaptureData& capture_data,
                          const absl::flat_hash_map<uint64_t, std::string>& key_to_string_map,
                          TimersIterator timers_iterator_begin, TimersIterator timers_iterator_end);

void WriteMessage(const google::protobuf::Message* message,
                  google::protobuf::io::CodedOutputStream* output);

namespace internal {

inline const std::string kRequiredCaptureVersion = "1.52";

orbit_client_protos::CaptureInfo GenerateCaptureInfo(
    const CaptureData& capture_data,
    const absl::flat_hash_map<uint64_t, std::string>& key_to_string_map);

template <class TimersIterator>
void Save(std::ostream& stream, const CaptureData& capture_data,
          const absl::flat_hash_map<uint64_t, std::string>& key_to_string_map,
          TimersIterator timers_iterator_begin, TimersIterator timers_iterator_end) {
  google::protobuf::io::OstreamOutputStream out_stream(&stream);
  google::protobuf::io::CodedOutputStream coded_output(&out_stream);

  orbit_client_protos::CaptureHeader header;
  header.set_version(kRequiredCaptureVersion);
  WriteMessage(&header, &coded_output);

  orbit_client_protos::CaptureInfo capture_info =
      GenerateCaptureInfo(capture_data, key_to_string_map);
  WriteMessage(&capture_info, &coded_output);

  // Timers
  for (auto it = timers_iterator_begin; it != timers_iterator_end; ++it) {
    WriteMessage(&(*it), &coded_output);
  }
}

}  // namespace internal

template <class TimersIterator>
ErrorMessageOr<void> Save(const std::string& filename, const CaptureData& capture_data,
                          const absl::flat_hash_map<uint64_t, std::string>& key_to_string_map,
                          TimersIterator timers_iterator_begin,
                          TimersIterator timers_iterator_end) {
  std::ofstream file(filename, std::ios::binary);
  if (file.fail()) {
    ERROR("Saving capture in \"%s\": %s", filename, "file.fail()");
    return ErrorMessage("Error opening the file for writing");
  }

  {
    SCOPE_TIMER_LOG(absl::StrFormat("Saving capture in \"%s\"", filename));
    internal::Save(file, capture_data, key_to_string_map, std::move(timers_iterator_begin),
                   std::move(timers_iterator_end));
  }

  return outcome::success();
}

}  // namespace capture_serializer

#endif  // ORBIT_CLIENT_MODEL_CAPTURE_SERIALIZER_H_
