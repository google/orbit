// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_CAPTURE_DATA_H_
#define ORBIT_CORE_CAPTURE_DATA_H_

#include <memory>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "capture_data.pb.h"

class CaptureData {
 public:
  explicit CaptureData(
      int32_t process_id, std::string process_name,
      std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>>
          selected_functions)
      : process_id_{process_id},
        process_name_{std::move(process_name)},
        selected_functions_{std::move(selected_functions)} {}

  explicit CaptureData() = default;
  CaptureData(const CaptureData& other) = default;
  CaptureData& operator=(const CaptureData& other) = default;
  CaptureData(CaptureData&& other) = default;
  CaptureData& operator=(CaptureData&& other) = default;

  [[nodiscard]] const std::vector<
      std::shared_ptr<orbit_client_protos::FunctionInfo>>&
  selected_functions() const {
    return selected_functions_;
  }

  [[nodiscard]] int32_t process_id() const { return process_id_; }

  [[nodiscard]] const std::string& process_name() const {
    return process_name_;
  }

  [[nodiscard]] const std::chrono::system_clock::time_point&
  capture_start_time() const {
    return capture_start_time_;
  }

  [[nodiscard]] const absl::flat_hash_map<
      uint64_t, orbit_client_protos::LinuxAddressInfo>&
  address_infos() const {
    return address_infos_;
  }

  void set_address_infos(
      absl::flat_hash_map<uint64_t, orbit_client_protos::LinuxAddressInfo>
          address_infos) {
    address_infos_ = std::move(address_infos);
  }

  [[nodiscard]] orbit_client_protos::LinuxAddressInfo* GetAddressInfo(
      uint64_t address);

  [[nodiscard]] const absl::flat_hash_map<int32_t, std::string>& thread_names()
      const {
    return thread_names_;
  }

  [[nodiscard]] const std::string& GetThreadName(int32_t thread_id) {
    return thread_names_[thread_id];
  }

  void set_thread_names(
      absl::flat_hash_map<int32_t, std::string> thread_names) {
    thread_names_ = std::move(thread_names);
  }

  void AddOrAssignThreadName(int32_t thread_id, std::string thread_name) {
    thread_names_.insert_or_assign(thread_id, std::move(thread_name));
  }

 private:
  absl::flat_hash_map<uint64_t, orbit_client_protos::LinuxAddressInfo>
      address_infos_;
  int32_t process_id_ = -1;
  std::string process_name_;
  // TODO(kuebler): make them raw pointers at some point
  std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>>
      selected_functions_;

  absl::flat_hash_map<int32_t, std::string> thread_names_;

  std::chrono::system_clock::time_point capture_start_time_ =
      std::chrono::system_clock::now();
};

#endif  // ORBIT_CORE_CAPTURE_DATA_H_
