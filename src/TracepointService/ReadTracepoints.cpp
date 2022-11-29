// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ReadTracepoints.h"

#include <absl/strings/str_format.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <system_error>
#include <vector>

#include "GrpcProtos/tracepoint.pb.h"
#include "OrbitBase/Result.h"

namespace fs = std::filesystem;

namespace orbit_tracepoint_service {

static const char* kLinuxTracingEventsDirectory = "/sys/kernel/debug/tracing/events/";

ErrorMessageOr<std::vector<orbit_grpc_protos::TracepointInfo>> ReadTracepoints() {
  std::vector<orbit_grpc_protos::TracepointInfo> result;

  std::error_code error;
  auto category_directory_iterator = fs::directory_iterator(kLinuxTracingEventsDirectory, error);
  if (error) {
    return ErrorMessage{absl::StrFormat("Unable to scan \"%s\" directory: %s",
                                        kLinuxTracingEventsDirectory, error.message())};
  }

  for (auto category_it = fs::begin(category_directory_iterator),
            category_end = fs::end(category_directory_iterator);
       category_it != category_end; category_it.increment(error)) {
    if (error) {
      return ErrorMessage{absl::StrFormat("Unable to scan \"%s\" directory: %s",
                                          kLinuxTracingEventsDirectory, error.message())};
    }

    const fs::path& category_path = category_it->path();
    bool outer_is_directory = fs::is_directory(category_path, error);
    if (error) {
      return ErrorMessage{
          absl::StrFormat("Unable to stat \"%s\": %s", category_path.string(), error.message())};
    }

    if (!outer_is_directory) continue;

    auto name_directory_iterator = fs::directory_iterator(category_path, error);
    if (error) {
      return ErrorMessage{absl::StrFormat("Unable to scan \"%s\" directory: %s",
                                          category_path.string(), error.message())};
    }

    for (auto it = fs::begin(name_directory_iterator), end = fs::end(name_directory_iterator);
         it != end; it.increment(error)) {
      if (error) {
        return ErrorMessage{absl::StrFormat("Unable to scan \"%s\" directory: %s",
                                            category_path.string(), error.message())};
      }

      bool inner_is_directory = it->is_directory(error);
      if (error) {
        return ErrorMessage{
            absl::StrFormat("Unable to stat \"%s\": %s", it->path().string(), error.message())};
      }

      if (!inner_is_directory) {
        continue;
      }

      orbit_grpc_protos::TracepointInfo tracepoint_info;
      tracepoint_info.set_name(it->path().filename());
      tracepoint_info.set_category(category_path.filename());
      result.emplace_back(tracepoint_info);
    }
  }

  return result;
}

}  // namespace orbit_tracepoint_service
