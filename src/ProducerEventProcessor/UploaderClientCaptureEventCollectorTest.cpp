// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/support/channel_arguments.h>
#include <gtest/gtest.h>

#include <thread>

#include "CaptureFile/CaptureFileOutputStream.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "OrbitBase/Logging.h"
#include "ProducerEventProcessor/UploaderClientCaptureEventCollector.h"

using orbit_grpc_protos::ClientCaptureEvent;

namespace orbit_producer_event_processor {

namespace {

constexpr const char* kAnswerString = "Test Interned String";
constexpr uint64_t kAnswerKey = 42;
constexpr size_t kUploadBufferSize = 100;

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
  void StartProduce(size_t event_count) {
    ASSERT_GE(event_count, 1);
    data_produce_thread_ = std::thread(&UploaderClientCaptureEventCollectorTest::ProduceCaptureData,
                                       this, event_count);
  }

  void StartUpload() {
    data_upload_thread_ =
        std::thread(&UploaderClientCaptureEventCollectorTest::UploadCaptureData, this);
  }

  void TearDown() override {
    ASSERT_TRUE(data_produce_thread_.joinable());
    data_produce_thread_.join();

    ASSERT_TRUE(data_upload_thread_.joinable());
    data_upload_thread_.join();
  }

  UploaderClientCaptureEventCollector collector_;
  size_t total_uploaded_data_bytes_ = 0;
  absl::Mutex produce_finished_mutex_;
  bool produce_finished_ ABSL_GUARDED_BY(produce_finished_mutex_) = false;
  absl::Mutex upload_finished_mutex_;
  bool upload_finished_ ABSL_GUARDED_BY(upload_finished_mutex_) = false;

 private:
  // Keep producing capture events until `event_count` events are produced. The last produced event
  // will always be the `CaptureFinishedEvent`.
  void ProduceCaptureData(size_t event_count) {
    while (event_count-- > 1) {
      constexpr uint64_t kProduceEventEveryMs = 10;
      std::this_thread::sleep_for(std::chrono::milliseconds(kProduceEventEveryMs));

      collector_.AddEvent(CreateInternedStringCaptureEvent(kAnswerKey, kAnswerString));
    }

    collector_.AddEvent(CreateCaptureFinishedEvent());

    absl::MutexLock lock{&produce_finished_mutex_};
    produce_finished_ = true;
  }

  void UploadCaptureData() {
    while (collector_.DetermineDataReadiness() !=
           orbit_capture_uploader::DataReadiness::kEndOfData) {
      constexpr uint64_t kUploadEventsEveryMs = 10;
      std::this_thread::sleep_for(std::chrono::milliseconds(kUploadEventsEveryMs));

      total_uploaded_data_bytes_ +=
          collector_.ReadIntoBuffer(upload_buffer_.data(), kUploadBufferSize);
    }

    absl::MutexLock lock{&upload_finished_mutex_};
    upload_finished_ = true;
  }

  std::thread data_upload_thread_;
  std::thread data_produce_thread_;
  std::array<char, kUploadBufferSize> upload_buffer_;
};

}  // namespace

TEST_F(UploaderClientCaptureEventCollectorTest, UploadAfterFinishingProducing) {
  constexpr size_t kEventCount = 10;
  StartProduce(kEventCount);

  {
    absl::MutexLock lock{&produce_finished_mutex_};
    produce_finished_mutex_.Await(absl::Condition(&produce_finished_));
  }
  StartUpload();

  {
    absl::MutexLock lock{&upload_finished_mutex_};
    upload_finished_mutex_.Await(absl::Condition(&upload_finished_));
  }
  EXPECT_EQ(collector_.GetTotalUploadedEventCount(), kEventCount);
  EXPECT_EQ(collector_.GetTotalUploadedDataBytes(), total_uploaded_data_bytes_);
}

TEST_F(UploaderClientCaptureEventCollectorTest, UploadWhileStillProducing) {
  constexpr size_t kEventCount = 10;
  StartProduce(kEventCount);
  StartUpload();

  {
    absl::MutexLock lock{&upload_finished_mutex_};
    upload_finished_mutex_.Await(absl::Condition(&upload_finished_));
  }
  EXPECT_EQ(collector_.GetTotalUploadedEventCount(), kEventCount);
  EXPECT_EQ(collector_.GetTotalUploadedDataBytes(), total_uploaded_data_bytes_);
}

}  // namespace orbit_producer_event_processor