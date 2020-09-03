// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_SERIALIZER_H_
#define ORBIT_GL_CAPTURE_SERIALIZER_H_

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>

#include <iosfwd>
#include <outcome.hpp>
#include <string>

#include "CaptureData.h"
#include "OrbitBase/Result.h"
#include "TimeGraph.h"
#include "capture_data.pb.h"

class CaptureSerializer {
 public:
  template <class TimersIterator>
  static ErrorMessageOr<void> Save(
      const std::string& filename, const CaptureData& capture_data,
      const absl::flat_hash_map<uint64_t, std::string>& key_to_string_map,
      TimersIterator timers_iterator_begin, TimersIterator timers_iterator_end);

  ErrorMessageOr<void> Load(std::istream& stream);
  ErrorMessageOr<void> Load(const std::string& filename);

  TimeGraph* time_graph_;

  static bool ReadMessage(google::protobuf::Message* message,
                          google::protobuf::io::CodedInputStream* input);
  static void WriteMessage(const google::protobuf::Message* message,
                           google::protobuf::io::CodedOutputStream* output);

 private:
  template <class TimersIterator>
  static void Save(std::ostream& stream, const orbit_client_protos::CaptureHeader& header,
                   const CaptureData& capture_data,
                   const absl::flat_hash_map<uint64_t, std::string>& key_to_string_map,
                   TimersIterator timers_iterator_begin, TimersIterator timers_iterator_end);

  static orbit_client_protos::CaptureInfo GenerateCaptureInfo(
      const CaptureData& capture_data,
      const absl::flat_hash_map<uint64_t, std::string>& key_to_string_map);

  [[nodiscard]] CaptureData GenerateCaptureData(
      const orbit_client_protos::CaptureInfo& capture_info);

  orbit_client_protos::CaptureHeader header_;

  static inline const std::string kRequiredCaptureVersion = "1.52";
};

template <class TimersIterator>
void CaptureSerializer::Save(std::ostream& stream, const orbit_client_protos::CaptureHeader& header,
                             const CaptureData& capture_data,
                             const absl::flat_hash_map<uint64_t, std::string>& key_to_string_map,
                             TimersIterator begin, TimersIterator end) {
  google::protobuf::io::OstreamOutputStream out_stream(&stream);
  google::protobuf::io::CodedOutputStream coded_output(&out_stream);

  WriteMessage(&header, &coded_output);

  orbit_client_protos::CaptureInfo capture_info =
      GenerateCaptureInfo(capture_data, key_to_string_map);
  WriteMessage(&capture_info, &coded_output);

  // Timers
  for (; begin != end; ++begin) {
    WriteMessage(&(*begin), &coded_output);
  }
}

template <class TimersIterator>
ErrorMessageOr<void> CaptureSerializer::Save(
    const std::string& filename, const CaptureData& capture_data,
    const absl::flat_hash_map<uint64_t, std::string>& key_to_string_map,
    TimersIterator timers_iterator_begin, TimersIterator timers_iterator_end) {
  orbit_client_protos::CaptureHeader header;
  header.set_version(kRequiredCaptureVersion);

  std::ofstream file(filename, std::ios::binary);
  if (file.fail()) {
    ERROR("Saving capture in \"%s\": %s", filename, "file.fail()");
    return ErrorMessage("Error opening the file for writing");
  }

  {
    SCOPE_TIMER_LOG(absl::StrFormat("Saving capture in \"%s\"", filename));

    Save(file, header, capture_data, key_to_string_map, std::move(timers_iterator_begin),
         std::move(timers_iterator_end));
  }

  return outcome::success();
}

#endif  // ORBIT_GL_CAPTURE_SERIALIZER_H_
