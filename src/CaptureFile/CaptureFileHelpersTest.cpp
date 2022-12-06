// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "CaptureFile/CaptureFile.h"
#include "CaptureFile/CaptureFileHelpers.h"
#include "CaptureFile/CaptureFileOutputStream.h"
#include "CaptureFile/CaptureFileSection.h"
#include "CaptureFile/ProtoSectionInputStream.h"
#include "ClientProtos/user_defined_capture_info.pb.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Result.h"
#include "TestUtils/TemporaryFile.h"
#include "TestUtils/TestUtils.h"

namespace orbit_capture_file {

using orbit_test_utils::HasNoError;

static constexpr const char* kAnswerString =
    "Answer to the Ultimate Question of Life, The Universe, and Everything";
static constexpr const char* kNotAnAnswerString = "Some odd number, not the answer.";
static constexpr uint64_t kAnswerKey = 42;
static constexpr uint64_t kNotAnAnswerKey = 43;

using orbit_grpc_protos::ClientCaptureEvent;

static ClientCaptureEvent CreateInternedStringCaptureEvent(uint64_t key, std::string str) {
  ClientCaptureEvent event;
  orbit_grpc_protos::InternedString* interned_string = event.mutable_interned_string();
  interned_string->set_key(key);
  interned_string->set_intern(std::move(str));
  return event;
}

TEST(CaptureFileHelpers, CreateCaptureFileAndWriteUserData) {
  auto temporary_file_or_error = orbit_test_utils::TemporaryFile::Create();
  ASSERT_THAT(temporary_file_or_error, HasNoError());
  orbit_test_utils::TemporaryFile temporary_file = std::move(temporary_file_or_error.value());

  const std::filesystem::path& file_path = temporary_file.file_path();
  temporary_file.CloseAndRemove();

  auto output_stream_or_error = CaptureFileOutputStream::Create(file_path);
  ASSERT_THAT(output_stream_or_error, HasNoError());

  std::unique_ptr<CaptureFileOutputStream> output_stream =
      std::move(output_stream_or_error.value());

  EXPECT_TRUE(output_stream->IsOpen());

  orbit_grpc_protos::ClientCaptureEvent event1 =
      CreateInternedStringCaptureEvent(kAnswerKey, kAnswerString);
  orbit_grpc_protos::ClientCaptureEvent event2 =
      CreateInternedStringCaptureEvent(kNotAnAnswerKey, kNotAnAnswerString);

  ASSERT_THAT(output_stream->WriteCaptureEvent(event1), HasNoError());
  ASSERT_THAT(output_stream->WriteCaptureEvent(event2), HasNoError());
  ASSERT_THAT(output_stream->Close(), HasNoError());

  {
    orbit_client_protos::UserDefinedCaptureInfo user_defined_capture_info;
    user_defined_capture_info.mutable_frame_tracks_info()->add_frame_track_function_ids(1);
    ASSERT_THAT(WriteUserData(file_path, user_defined_capture_info), HasNoError());
  }

  {
    auto capture_file_or_error = CaptureFile::OpenForReadWrite(file_path);
    ASSERT_THAT(capture_file_or_error, HasNoError());
    auto capture_file = std::move(capture_file_or_error.value());
    std::optional<uint64_t> section_number = capture_file->FindSectionByType(kSectionTypeUserData);
    ASSERT_TRUE(section_number.has_value());
    auto input_stream = capture_file->CreateProtoSectionInputStream(section_number.value());
    ASSERT_NE(input_stream.get(), nullptr);
    orbit_client_protos::UserDefinedCaptureInfo user_defined_capture_info;
    ASSERT_THAT(input_stream->ReadMessage(&user_defined_capture_info), HasNoError());
    ASSERT_EQ(user_defined_capture_info.frame_tracks_info().frame_track_function_ids_size(), 1);
    EXPECT_EQ(user_defined_capture_info.frame_tracks_info().frame_track_function_ids(0), 1);
  }

  // Now update it
  {
    orbit_client_protos::UserDefinedCaptureInfo user_defined_capture_info;
    user_defined_capture_info.mutable_frame_tracks_info()->add_frame_track_function_ids(1);
    user_defined_capture_info.mutable_frame_tracks_info()->add_frame_track_function_ids(2);
    user_defined_capture_info.mutable_frame_tracks_info()->add_frame_track_function_ids(3);
    ASSERT_THAT(WriteUserData(file_path, user_defined_capture_info), HasNoError());
  }

  {
    auto capture_file_or_error = CaptureFile::OpenForReadWrite(file_path);
    ASSERT_THAT(capture_file_or_error, HasNoError());
    auto capture_file = std::move(capture_file_or_error.value());
    std::optional<uint64_t> section_number = capture_file->FindSectionByType(kSectionTypeUserData);
    ASSERT_TRUE(section_number.has_value());
    auto input_stream = capture_file->CreateProtoSectionInputStream(section_number.value());
    ASSERT_NE(input_stream.get(), nullptr);
    orbit_client_protos::UserDefinedCaptureInfo user_defined_capture_info;
    ASSERT_THAT(input_stream->ReadMessage(&user_defined_capture_info), HasNoError());
    ASSERT_EQ(user_defined_capture_info.frame_tracks_info().frame_track_function_ids_size(), 3);
    EXPECT_EQ(user_defined_capture_info.frame_tracks_info().frame_track_function_ids(0), 1);
    EXPECT_EQ(user_defined_capture_info.frame_tracks_info().frame_track_function_ids(1), 2);
    EXPECT_EQ(user_defined_capture_info.frame_tracks_info().frame_track_function_ids(2), 3);
  }
}

}  // namespace orbit_capture_file