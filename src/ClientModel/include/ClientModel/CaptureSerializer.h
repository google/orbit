// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_MODEL_CAPTURE_SERIALIZER_H_
#define CLIENT_MODEL_CAPTURE_SERIALIZER_H_

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

namespace orbit_client_model::capture_serializer {

void WriteMessage(const google::protobuf::Message* message,
                  google::protobuf::io::CodedOutputStream* output);

std::string GenerateCaptureFileName(std::string_view process_name, absl::Time time,
                                    std::string_view suffix = "");

void IncludeOrbitExtensionInFile(std::string& file_name);

namespace internal {

orbit_client_protos::CaptureInfo GenerateCaptureInfo(
    const CaptureData& capture_data,
    const absl::flat_hash_map<uint64_t, std::string>& key_to_string_map);

}  // namespace internal

}  // namespace orbit_client_model::capture_serializer

#endif  // CLIENT_MODEL_CAPTURE_SERIALIZER_H_
