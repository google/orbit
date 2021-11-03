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
#include "ProducerEventProcessor/UploaderClientCaptureEventCollector.h"
#include "capture.pb.h"
#include "services.grpc.pb.h"

using orbit_grpc_protos::ClientCaptureEvent;

namespace orbit_producer_event_processor {

namespace {

static constexpr const char* kAnswerString = "Test Interned String";
static constexpr uint64_t kAnswerKey = 42;

ClientCaptureEvent CreateInternedStringCaptureEvent(uint64_t key, const std::string& str) {
  ClientCaptureEvent event;
  event.mutable_interned_string()->set_key(key);
  event.mutable_interned_string()->set_intern(str);
  return event;
}

ClientCaptureEvent CreateCaptureFinishedEvent() {
  ClientCaptureEvent event;
  event.mutable_capture_finished()->set_status(orbit_grpc_protos::CaptureFinished::kSuccessful);
  return event;
}

class UploaderClientCaptureEventCollectorTest : public testing::Test {
 protected:
  // Set `early_stop_event_count` to be >= 2 if we want to call `Stop()` early with producing only
  // `early_stop_event_count` events. Here we require ">= 2" as an early stop will create at
  // least two capture events: one InternedStringCaptureEvent created when stop_called_ becomes
  // true, and one `CaptureFinishEvent`.
  void Start(size_t event_count, uint64_t produce_event_every_ms, uint64_t upload_event_every_ms,
             size_t early_stop_event_count = 0) {
    event_count_ = event_count;
    produce_event_every_ms_ = produce_event_every_ms;
    upload_event_every_ms_ = upload_event_every_ms;
    early_stop_event_count_ = early_stop_event_count;

    data_produce_thread_ =
        std::thread(&UploaderClientCaptureEventCollectorTest::ProduceCaptureData, this);
    data_upload_thread_ =
        std::thread(&UploaderClientCaptureEventCollectorTest::UploadCaptureData, this);
  }

  void Stop() {
    absl::MutexLock lock{&stop_called_mutex_};
    stop_called_ = true;
  }

  void TearDown() override {
    {
      absl::MutexLock lock{&stop_called_mutex_};
      CHECK(stop_called_);
    }

    CHECK(data_produce_thread_.joinable());
    data_produce_thread_.join();

    CHECK(data_upload_thread_.joinable());
    data_upload_thread_.join();
  }

  UploaderClientCaptureEventCollector collector_;
  size_t total_uploaded_data_bytes_ = 0;

  absl::Mutex upload_finished_mutex_;
  bool upload_finished_ ABSL_GUARDED_BY(upload_finished_mutex_) = false;
  absl::Mutex early_stop_mutex_;
  bool early_stop_ ABSL_GUARDED_BY(early_stop_mutex_) = false;

 private:
  // Keep producing capture events until stop is called or `event_count_` events are produced with a
  // specified produce duration. The last produced event will always be the capture finish event.
  void ProduceCaptureData() {
    size_t added_event_count = 0;
    bool need_check_early_stop = (early_stop_event_count_ >= 2);
    bool stopped = false;
    while (added_event_count < event_count_ - 1 && !stopped) {
      if (need_check_early_stop && added_event_count + 2 >= early_stop_event_count_) {
        absl::MutexLock lock{&early_stop_mutex_};
        early_stop_ = true;
        need_check_early_stop = false;
      }

      stop_called_mutex_.LockWhenWithTimeout(absl::Condition(&stop_called_),
                                             absl::Milliseconds(produce_event_every_ms_));
      collector_.AddEvent(CreateInternedStringCaptureEvent(kAnswerKey, kAnswerString));
      stopped = stop_called_;
      stop_called_mutex_.Unlock();

      ++added_event_count;
    }

    collector_.AddEvent(CreateCaptureFinishedEvent());

    Stop();
  }

  void UploadCaptureData() {
    while (collector_.GetDataReadiness() != orbit_capture_uploader::DataReadiness::kEndOfData) {
      std::this_thread::sleep_for(std::chrono::milliseconds(upload_event_every_ms_));

      const std::vector<unsigned char>& upload_data_buffer = collector_.GetUploadDataBuffer();
      total_uploaded_data_bytes_ += upload_data_buffer.size();
      collector_.RefreshUploadDataBuffer();
    }

    absl::MutexLock lock{&upload_finished_mutex_};
    upload_finished_ = true;
  }

  absl::Mutex stop_called_mutex_;
  bool stop_called_ ABSL_GUARDED_BY(stop_called_mutex_) = false;

  size_t event_count_ = 0;
  size_t early_stop_event_count_;
  uint64_t produce_event_every_ms_;
  uint64_t upload_event_every_ms_;
  std::thread data_upload_thread_;
  std::thread data_produce_thread_;
};

}  // namespace

TEST_F(UploaderClientCaptureEventCollectorTest, EarlyStop) {
  constexpr size_t kEventCount = 10;
  constexpr uint64_t kProduceEventEveryMs = 50;
  constexpr uint64_t kUploadEventEveryMs = 5;
  constexpr size_t kEarlyStopEventCount = 4;
  Start(kEventCount, kProduceEventEveryMs, kUploadEventEveryMs, kEarlyStopEventCount);

  // Stop producing data early
  {
    absl::MutexLock lock{&early_stop_mutex_};
    early_stop_mutex_.Await(absl::Condition(&early_stop_));
  }
  Stop();

  {
    absl::MutexLock lock{&upload_finished_mutex_};
    upload_finished_mutex_.Await(absl::Condition(&upload_finished_));
  }
  EXPECT_EQ(collector_.GetTotalUploadedEventCount(), kEarlyStopEventCount);
  EXPECT_EQ(collector_.GetTotalUploadedDataBytes(), total_uploaded_data_bytes_);
}

TEST_F(UploaderClientCaptureEventCollectorTest, SlowProduceAndFastUpload) {
  constexpr size_t kEventCount = 10;
  constexpr uint64_t kSlowProduceEventEveryMs = 50;
  constexpr uint64_t kFastUploadEventEveryMs = 5;
  Start(kEventCount, kSlowProduceEventEveryMs, kFastUploadEventEveryMs);

  {
    absl::MutexLock lock{&upload_finished_mutex_};
    upload_finished_mutex_.Await(absl::Condition(&upload_finished_));
  }
  EXPECT_EQ(collector_.GetTotalUploadedEventCount(), kEventCount);
  EXPECT_EQ(collector_.GetTotalUploadedDataBytes(), total_uploaded_data_bytes_);
}

TEST_F(UploaderClientCaptureEventCollectorTest, FastProduceAndSlowUpload) {
  constexpr size_t kEventCount = 10;
  constexpr uint64_t kFastProduceEventEveryMs = 5;
  constexpr uint64_t kSlowUploadEventEveryMs = 50;
  Start(kEventCount, kFastProduceEventEveryMs, kSlowUploadEventEveryMs);

  {
    absl::MutexLock lock{&upload_finished_mutex_};
    upload_finished_mutex_.Await(absl::Condition(&upload_finished_));
  }
  EXPECT_EQ(collector_.GetTotalUploadedEventCount(), kEventCount);
  EXPECT_EQ(collector_.GetTotalUploadedDataBytes(), total_uploaded_data_bytes_);
}

}  // namespace orbit_producer_event_processor