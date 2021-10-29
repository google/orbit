// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/support/channel_arguments.h>
#include <gtest/gtest.h>

#include <thread>

#include "CaptureFile/CaptureFileOutputStream.h"
#include "OrbitBase/Logging.h"
#include "ProducerEventProcessor/UploaderCaptureEventCollector.h"
#include "capture.pb.h"
#include "services.grpc.pb.h"

using orbit_grpc_protos::ClientCaptureEvent;

namespace orbit_producer_event_processor {

namespace {

static constexpr const char* kAnswerString = "Test Interned String";
static constexpr uint64_t kAnswerKey = 42;

static ClientCaptureEvent CreateInternedStringCaptureEvent(uint64_t key, const std::string& str) {
  ClientCaptureEvent event;
  event.mutable_interned_string()->set_key(key);
  event.mutable_interned_string()->set_intern(str);
  return event;
}

static ClientCaptureEvent CreateCaptureFinishedEvent() {
  ClientCaptureEvent event;
  event.mutable_capture_finished()->set_status(orbit_grpc_protos::CaptureFinished::kSuccessful);
  return event;
}

class UploaderCaptureEventCollectorTest : public testing::Test {
 public:
  void Start(uint64_t event_count, uint64_t produce_duration_ms, uint64_t upload_duration_ms) {
    event_count_ = event_count;
    produce_duration_ms_ = produce_duration_ms;
    upload_duration_ms_ = upload_duration_ms;

    uint64_t kWaitDurationMs = 20;
    wait_all_data_uploaded_duration_ms_ =
        kWaitDurationMs + upload_duration_ms_ + produce_duration_ms_ * event_count_;

    data_produce_thread_ =
        std::thread(&UploaderCaptureEventCollectorTest::ProduceCaptureData, this);
    data_upload_thread_ = std::thread(&UploaderCaptureEventCollectorTest::UploadCaptureData, this);
  }

  void Stop() {
    absl::MutexLock lock{&mutex_};
    stop_called_ = true;
  }

 protected:
  void TearDown() override {
    absl::MutexLock lock{&mutex_};
    CHECK(stop_called_);

    if (data_produce_thread_.joinable()) data_produce_thread_.join();
    if (data_upload_thread_.joinable()) data_upload_thread_.join();
  }

  UploaderCaptureEventCollector collector_;
  size_t total_uploaded_data_bytes_ = 0;
  uint64_t wait_all_data_uploaded_duration_ms_;

 private:
  // Keep producing capture event until stop is called or `event_count_` events are produced with a
  // specified produce duration. The last produced event will always be the capture finish event.
  void ProduceCaptureData() {
    size_t event_count = event_count_;
    bool stopped = false;
    while (event_count-- > 1 && !stopped) {
      mutex_.LockWhenWithTimeout(absl::Condition(&stop_called_),
                                 absl::Milliseconds(produce_duration_ms_));
      collector_.AddEvent(CreateInternedStringCaptureEvent(kAnswerKey, kAnswerString));
      stopped = stop_called_;
      mutex_.Unlock();
    }

    collector_.AddEvent(CreateCaptureFinishedEvent());

    collector_.Stop();
    Stop();
  }

  void UploadCaptureData() {
    const std::vector<unsigned char>& upload_data_buffer_ = collector_.GetUploadDataBuffer();

    while (collector_.GetDataReadiness() != orbit_capture_uploader::DataReadiness::kEndOfData) {
      std::this_thread::sleep_for(std::chrono::milliseconds(upload_duration_ms_));
      total_uploaded_data_bytes_ += upload_data_buffer_.size();
      collector_.RefreshUploadDataBuffer();
    }
  }

  absl::Mutex mutex_;
  bool stop_called_ ABSL_GUARDED_BY(mutex_) = false;
  size_t event_count_ = 0;
  uint64_t produce_duration_ms_;
  uint64_t upload_duration_ms_;
  std::thread data_upload_thread_;
  std::thread data_produce_thread_;
};

}  // namespace

TEST_F(UploaderCaptureEventCollectorTest, EarlyStop) {
  constexpr uint64_t kEventCount = 10;
  constexpr uint64_t kProduceDurationMs = 50;
  constexpr uint64_t kUploadDurationMs = 5;
  constexpr uint64_t kEarlyStopMs = 120;
  constexpr uint64_t kActualProducedEventCount = 4;
  Start(kEventCount, kProduceDurationMs, kUploadDurationMs);

  // Stop producing data early and wait until all data is uploaded
  std::this_thread::sleep_for(std::chrono::milliseconds(kEarlyStopMs));
  Stop();
  std::this_thread::sleep_for(
      std::chrono::milliseconds(wait_all_data_uploaded_duration_ms_ - kEarlyStopMs));
  EXPECT_EQ(collector_.GetTotalUploadedEventCount(), kActualProducedEventCount);
  EXPECT_EQ(collector_.GetTotalUploadedDataBytes(), total_uploaded_data_bytes_);
}

TEST_F(UploaderCaptureEventCollectorTest, SlowProduceAndFastUpload) {
  constexpr uint64_t kEventCount = 10;
  constexpr uint64_t kSlowProduceDurationMs = 50;
  constexpr uint64_t kFastUploadDurationMs = 5;
  Start(kEventCount, kSlowProduceDurationMs, kFastUploadDurationMs);

  std::this_thread::sleep_for(std::chrono::milliseconds(wait_all_data_uploaded_duration_ms_));
  EXPECT_EQ(collector_.GetTotalUploadedEventCount(), kEventCount);
  EXPECT_EQ(collector_.GetTotalUploadedDataBytes(), total_uploaded_data_bytes_);
}

TEST_F(UploaderCaptureEventCollectorTest, FastProduceAndSlowUpload) {
  constexpr uint64_t kEventCount = 10;
  constexpr uint64_t kFastProduceDurationMs = 5;
  constexpr uint64_t kSlowUploadDurationMs = 50;
  Start(kEventCount, kFastProduceDurationMs, kSlowUploadDurationMs);

  std::this_thread::sleep_for(std::chrono::milliseconds(wait_all_data_uploaded_duration_ms_));
  EXPECT_EQ(collector_.GetTotalUploadedEventCount(), kEventCount);
  EXPECT_EQ(collector_.GetTotalUploadedDataBytes(), total_uploaded_data_bytes_);
}

}  // namespace orbit_producer_event_processor