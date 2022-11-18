// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_PAGE_FAULTS_INFO_H_
#define CLIENT_DATA_PAGE_FAULTS_INFO_H_

#include <cstdint>

#include "GrpcProtos/Constants.h"

namespace orbit_client_data {

struct PageFaultsInfo {
  uint64_t timestamp_ns;
  int64_t system_page_faults;
  int64_t system_major_page_faults;
  uint64_t cgroup_name_hash;
  int64_t cgroup_page_faults;
  int64_t cgroup_major_page_faults;
  int64_t process_minor_page_faults;
  int64_t process_major_page_faults;

  [[nodiscard]] bool HasMajorPageFaultsInfo() const {
    return system_major_page_faults != orbit_grpc_protos::kMissingInfo &&
           cgroup_major_page_faults != orbit_grpc_protos::kMissingInfo &&
           process_major_page_faults != orbit_grpc_protos::kMissingInfo;
  }

  [[nodiscard]] bool HasMinorPageFaultsInfo() const {
    return system_page_faults != orbit_grpc_protos::kMissingInfo &&
           system_major_page_faults != orbit_grpc_protos::kMissingInfo &&
           cgroup_page_faults != orbit_grpc_protos::kMissingInfo &&
           cgroup_major_page_faults != orbit_grpc_protos::kMissingInfo &&
           process_minor_page_faults != orbit_grpc_protos::kMissingInfo;
  }
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_PAGE_FAULTS_INFO_H_
