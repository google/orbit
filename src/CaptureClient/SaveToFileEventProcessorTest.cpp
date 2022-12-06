// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "CaptureClient/CaptureEventProcessor.h"
#include "CaptureFile/CaptureFile.h"
#include "CaptureFile/CaptureFileSection.h"
#include "CaptureFile/ProtoSectionInputStream.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Result.h"
#include "TestUtils/TemporaryFile.h"
#include "TestUtils/TestUtils.h"

namespace orbit_capture_client {

using orbit_capture_file::CaptureFile;
using orbit_grpc_protos::CaptureFinished;
using orbit_grpc_protos::ClientCaptureEvent;
using orbit_test_utils::HasNoError;
using orbit_test_utils::HasValue;
using orbit_test_utils::TemporaryFile;

static ClientCaptureEvent CreateInternedStringEvent(uint64_t key, const char* intern) {
  ClientCaptureEvent event;
  event.mutable_interned_string()->set_key(key);
  event.mutable_interned_string()->set_intern(intern);

  return event;
}

static ClientCaptureEvent CreateCaptureFinishedEvent() {
  ClientCaptureEvent event;
  event.mutable_capture_finished()->set_status(CaptureFinished::kSuccessful);
  return event;
}

TEST(SaveToFileEventProcessor, SaveAndLoadSimpleCapture) {
  auto temporary_file_or_error = TemporaryFile::Create();
  ASSERT_TRUE(temporary_file_or_error.has_value()) << temporary_file_or_error.error().message();
  TemporaryFile temporary_file = std::move(temporary_file_or_error.value());
  absl::flat_hash_set<uint64_t> frame_track_function_ids;

  auto error_handler = [](const ErrorMessage& error) { FAIL() << error.message(); };

  temporary_file.CloseAndRemove();

  auto capture_event_processor_or_error =
      CaptureEventProcessor::CreateSaveToFileProcessor(temporary_file.file_path(), error_handler);
  ASSERT_TRUE(capture_event_processor_or_error.has_value())
      << capture_event_processor_or_error.error().message();

  std::unique_ptr<CaptureEventProcessor> capture_event_processor =
      std::move(capture_event_processor_or_error.value());
  capture_event_processor->ProcessEvent(CreateInternedStringEvent(1, "1"));
  capture_event_processor->ProcessEvent(CreateInternedStringEvent(2, "2"));
  capture_event_processor->ProcessEvent(CreateInternedStringEvent(3, "3"));
  capture_event_processor->ProcessEvent(CreateCaptureFinishedEvent());

  capture_event_processor.reset();

  auto capture_file_or_error = CaptureFile::OpenForReadWrite(temporary_file.file_path());
  ASSERT_THAT(capture_event_processor_or_error, HasValue());
  auto capture_file = std::move(capture_file_or_error.value());

  auto capture_section_input_stream = capture_file->CreateCaptureSectionInputStream();

  {
    ClientCaptureEvent event;
    ASSERT_THAT(capture_section_input_stream->ReadMessage(&event), HasNoError());
    ASSERT_EQ(event.event_case(), ClientCaptureEvent::kInternedString);
    EXPECT_EQ(event.interned_string().key(), 1);
    EXPECT_EQ(event.interned_string().intern(), "1");
  }

  {
    ClientCaptureEvent event;
    ASSERT_THAT(capture_section_input_stream->ReadMessage(&event), HasNoError());
    ASSERT_EQ(event.event_case(), ClientCaptureEvent::kInternedString);
    EXPECT_EQ(event.interned_string().key(), 2);
    EXPECT_EQ(event.interned_string().intern(), "2");
  }

  {
    ClientCaptureEvent event;
    ASSERT_THAT(capture_section_input_stream->ReadMessage(&event), HasNoError());
    ASSERT_EQ(event.event_case(), ClientCaptureEvent::kInternedString);
    EXPECT_EQ(event.interned_string().key(), 3);
    EXPECT_EQ(event.interned_string().intern(), "3");
  }

  {
    ClientCaptureEvent event;
    ASSERT_THAT(capture_section_input_stream->ReadMessage(&event), HasNoError());
    ASSERT_EQ(event.event_case(), ClientCaptureEvent::kCaptureFinished);
    EXPECT_EQ(event.capture_finished().status(), CaptureFinished::kSuccessful);
  }

  const auto& sections = capture_file->GetSectionList();
  EXPECT_EQ(sections.size(), 0);

  std::optional<size_t> user_data_section =
      capture_file->FindSectionByType(orbit_capture_file::kSectionTypeUserData);
  EXPECT_FALSE(user_data_section.has_value());
}

}  // namespace orbit_capture_client