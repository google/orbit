// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <vector>

#include "CaptureClient/CaptureEventProcessor.h"
#include "CaptureFile/CaptureFile.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/TemporaryFile.h"
#include "OrbitBase/TestUtils.h"

namespace orbit_capture_client {

using orbit_base::HasValue;
using orbit_base::TemporaryFile;
using orbit_capture_file::CaptureFile;
using orbit_client_protos::UserDefinedCaptureInfo;
using orbit_grpc_protos::CaptureFinished;
using orbit_grpc_protos::ClientCaptureEvent;

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

TEST(SaveToFileCaptureEventProcessor, SaveAndLoadSimpleCaptureWithFrameTracks) {
  auto temporary_file_or_error = TemporaryFile::Create();
  ASSERT_TRUE(temporary_file_or_error.has_value()) << temporary_file_or_error.error().message();
  TemporaryFile temporary_file = std::move(temporary_file_or_error.value());
  absl::flat_hash_set<uint64_t> frame_track_function_ids;
  constexpr uint64_t kFrameTrackFunctionId = 17;
  frame_track_function_ids.insert(kFrameTrackFunctionId);

  auto error_handler = [](const ErrorMessage& error) { FAIL() << error.message(); };

  auto capture_event_processor_or_error = CaptureEventProcessor::CreateSaveToFileProcessor(
      temporary_file.file_path(), frame_track_function_ids, error_handler);
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
    auto event_or_error = capture_section_input_stream->ReadEvent();
    ASSERT_THAT(event_or_error, HasValue());
    const auto& event = event_or_error.value();
    ASSERT_EQ(event.event_case(), ClientCaptureEvent::kInternedString);
    EXPECT_EQ(event.interned_string().key(), 1);
    EXPECT_EQ(event.interned_string().intern(), "1");
  }

  {
    auto event_or_error = capture_section_input_stream->ReadEvent();
    ASSERT_THAT(event_or_error, HasValue());
    const auto& event = event_or_error.value();
    ASSERT_EQ(event.event_case(), ClientCaptureEvent::kInternedString);
    EXPECT_EQ(event.interned_string().key(), 2);
    EXPECT_EQ(event.interned_string().intern(), "2");
  }

  {
    auto event_or_error = capture_section_input_stream->ReadEvent();
    ASSERT_THAT(event_or_error, HasValue());
    const auto& event = event_or_error.value();
    ASSERT_EQ(event.event_case(), ClientCaptureEvent::kInternedString);
    EXPECT_EQ(event.interned_string().key(), 3);
    EXPECT_EQ(event.interned_string().intern(), "3");
  }

  {
    auto event_or_error = capture_section_input_stream->ReadEvent();
    ASSERT_THAT(event_or_error, HasValue());
    const auto& event = event_or_error.value();
    ASSERT_EQ(event.event_case(), ClientCaptureEvent::kCaptureFinished);
    EXPECT_EQ(event.capture_finished().status(), CaptureFinished::kSuccessful);
  }

  const auto& sections = capture_file->GetSectionList();
  ASSERT_GE(sections.size(), 1);

  std::optional<size_t> user_data_section;
  for (size_t i = 0; i < sections.size(); i++) {
    if (sections[i].type == orbit_capture_file::kSectionTypeUserData) {
      user_data_section = i;
      break;
    }
  }

  ASSERT_TRUE(user_data_section.has_value());

  std::vector<uint8_t> buf(sections[user_data_section.value()].size);
  ASSERT_THAT(capture_file->ReadFromSection(user_data_section.value(), 0, buf.data(), buf.size()),
              HasValue());

  UserDefinedCaptureInfo capture_info;
  capture_info.ParseFromArray(buf.data(), buf.size());

  ASSERT_EQ(capture_info.frame_tracks_info().frame_track_function_ids_size(), 1);
  EXPECT_EQ(capture_info.frame_tracks_info().frame_track_function_ids(0), kFrameTrackFunctionId);
}

}  // namespace orbit_capture_client