// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_CAPTURE_DATA_HOLDER_H_
#define CLIENT_DATA_CAPTURE_DATA_HOLDER_H_

#include <functional>
#include <memory>
#include <optional>

#include "ClientData/CaptureData.h"
#include "OrbitBase/Logging.h"

namespace orbit_client_data {

class CaptureDataHolder {
 public:
  CaptureDataHolder() = default;

  CaptureDataHolder(CaptureDataHolder&) = delete;
  CaptureDataHolder& operator=(const CaptureDataHolder& other) = delete;

  CaptureDataHolder(CaptureDataHolder&&) = default;
  CaptureDataHolder& operator=(CaptureDataHolder&& other) = default;

  virtual ~CaptureDataHolder() = default;

  // TODO(b/234095077): The two methods should not return ref, as capture_data_ doesn't
  // necessarily store a value. They should either return a pointer, or
  // `std::optional<std::reference_wrapper<CaptureData>>`
  [[nodiscard]] virtual const CaptureData& GetCaptureData() const {
    ORBIT_CHECK(HasCaptureData());
    return *capture_data_;
  }
  // CallstackDataView needs OrbitApp::GetMutableCaptureData()
  [[nodiscard]] virtual CaptureData& GetMutableCaptureData() {
    ORBIT_CHECK(HasCaptureData());
    return *capture_data_;
  }
  [[nodiscard]] const CaptureData* GetCaptureDataPointer() const {
    ORBIT_CHECK(HasCaptureData());
    return capture_data_.get();
  }

  [[nodiscard]] std::optional<ScopeId> ProvideScopeId(
      const orbit_client_protos::TimerInfo& timer_info) const {
    if (capture_data_ == nullptr) return std::nullopt;
    return capture_data_->ProvideScopeId(timer_info);
  }

  [[nodiscard]] virtual bool HasCaptureData() const { return static_cast<bool>(capture_data_); }

 protected:
  void ConstructCaptureData(const orbit_grpc_protos::CaptureStarted& capture_started,
                            std::optional<std::filesystem::path> file_path,
                            absl::flat_hash_set<uint64_t> frame_track_function_ids,
                            CaptureData::DataSource data_source,
                            const ModuleIdentifierProvider* module_identifier_provider) {
    capture_data_ = std::make_unique<orbit_client_data::CaptureData>(
        capture_started, std::move(file_path), std::move(frame_track_function_ids), data_source,
        module_identifier_provider);
  }

  void ResetCaptureData() { capture_data_.reset(); }

 private:
  // TODO(b/166767590): This is mostly written during capture by the capture thread on the
  // CaptureListener parts of App, but may be read also during capturing by all threads.
  // Currently, it is not properly synchronized (and thus it can't live at DataManager).
  std::unique_ptr<orbit_client_data::CaptureData> capture_data_;
};
}  // namespace orbit_client_data

#endif  // CLIENT_DATA_CAPTURE_DATA_HOLDER_H_