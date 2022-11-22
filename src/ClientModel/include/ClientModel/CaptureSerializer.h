// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_MODEL_CAPTURE_SERIALIZER_H_
#define CLIENT_MODEL_CAPTURE_SERIALIZER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/time/time.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>
#include <stdint.h>

#include <filesystem>
#include <string>
#include <string_view>

#include "ClientData/CaptureData.h"
#include "ClientProtos/capture_data.pb.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

namespace orbit_client_model_internal {
std::string FormatTimeWithUnderscores(absl::Time time);
}  // namespace orbit_client_model_internal

namespace orbit_client_model::capture_serializer {

std::string GenerateCaptureFileName(std::string_view process_name, absl::Time time,
                                    std::string_view suffix = "");

void IncludeOrbitExtensionInFile(std::string& file_name);

}  // namespace orbit_client_model::capture_serializer

#endif  // CLIENT_MODEL_CAPTURE_SERIALIZER_H_
